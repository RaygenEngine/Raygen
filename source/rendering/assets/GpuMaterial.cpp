#include "pch.h"
#include "GpuMaterial.h"

#include "assets/pods/Material.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Renderer.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/passes/DepthmapPass.h"


using namespace vl;

Material::Gpu::Gpu(PodHandle<Material> podHandle)
	: GpuAssetTemplate(podHandle)
{
	// CHECK: upload once for now (not dynamic changes)
	materialUBO = std::make_unique<RUboBuffer<UBO_Material>>(vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	Update({}); // NOTE: Virtual function call in constructor will not call subclasses overrides, thats why we
				// explicitly mark this function as final in the header
}

void Material::Gpu::Update(const AssetUpdateInfo& info)
{
	auto data = podHandle.Lock();

	matData.baseColorFactor = data->baseColorFactor;
	matData.emissiveFactor = glm::vec4(data->emissiveFactor, 1.0);
	matData.metallicFactor = data->metallicFactor;
	matData.roughnessFactor = data->roughnessFactor;
	matData.normalScale = data->normalScale;
	matData.occlusionStrength = data->occlusionStrength;

	// alpha mask
	matData.alphaCutoff = data->alphaCutoff;
	matData.mask = data->alphaMode == Material::AlphaMode::Mask;

	baseColorSampler = GpuAssetManager->GetGpuHandle(data->baseColorSampler);
	baseColorImage = GpuAssetManager->GetGpuHandle(data->baseColorImage);

	metallicRoughnessSampler = GpuAssetManager->GetGpuHandle(data->metallicRoughnessSampler);
	metallicRoughnessImage = GpuAssetManager->GetGpuHandle(data->metallicRoughnessImage);

	occlusionSampler = GpuAssetManager->GetGpuHandle(data->occlusionSampler);
	occlusionImage = GpuAssetManager->GetGpuHandle(data->occlusionImage);

	normalSampler = GpuAssetManager->GetGpuHandle(data->normalSampler);
	normalImage = GpuAssetManager->GetGpuHandle(data->normalImage);

	emissiveSampler = GpuAssetManager->GetGpuHandle(data->emissiveSampler);
	emissiveImage = GpuAssetManager->GetGpuHandle(data->emissiveImage);


	materialUBO->UploadData(matData);

	// TODO: descriptors
	descriptorSet = Layouts->regularMaterialDescLayout.GetDescriptorSet();

	// material uniform sets CHECK: (those buffers should be set again when material changes)
	vk::DescriptorBufferInfo bufferInfo{};

	bufferInfo
		.setBuffer(*materialUBO) //
		.setOffset(0u)
		.setRange(sizeof(UBO_Material));
	vk::WriteDescriptorSet descriptorWrite{};

	descriptorWrite
		.setDstSet(descriptorSet) //
		.setDstBinding(0u)
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1u)
		.setPBufferInfo(&bufferInfo)
		.setPImageInfo(nullptr)
		.setPTexelBufferView(nullptr);

	Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);


	// images (material)

	auto UpdateImageSamplerInDescriptorSet
		= [&](GpuHandle<Sampler> sampler, GpuHandle<Image> image, uint32 dstBinding) {
			  auto& sam = sampler.Lock();
			  auto& img = image.Lock();

			  vk::DescriptorImageInfo imageInfo{};
			  imageInfo
				  .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
				  .setImageView(img.image->GetView())
				  .setSampler(sam.sampler.get());

			  vk::WriteDescriptorSet descriptorWrite{};
			  descriptorWrite
				  .setDstSet(descriptorSet) //
				  .setDstBinding(dstBinding)
				  .setDstArrayElement(0u)
				  .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				  .setDescriptorCount(1u)
				  .setPBufferInfo(nullptr)
				  .setPImageInfo(&imageInfo)
				  .setPTexelBufferView(nullptr);

			  // PERF: Use a single descriptor update
			  Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
		  };

	UpdateImageSamplerInDescriptorSet(baseColorSampler, baseColorImage, 1u);
	UpdateImageSamplerInDescriptorSet(metallicRoughnessSampler, metallicRoughnessImage, 2u);
	UpdateImageSamplerInDescriptorSet(occlusionSampler, occlusionImage, 3u);
	UpdateImageSamplerInDescriptorSet(normalSampler, normalImage, 4u);
	UpdateImageSamplerInDescriptorSet(emissiveSampler, emissiveImage, 5u);

	// WIP: MATERIAL INSTANCE PART
	if (!data->wip_InstanceOverride.IsDefault()) {
		wip_UpdateMat();
	}
}

struct PC {
	glm::mat4 modelMat;
	glm::mat4 normalMat;
};

struct PushConstantShadow {
	glm::mat4 mvp;
};

void Material::Gpu::wip_UpdateMat()
{
	auto data = podHandle.Lock();

	ClearDependencies();
	AddDependency(data->wip_InstanceOverride);
	auto matInst = data->wip_InstanceOverride.Lock();
	auto matArch = matInst->archetype.Lock();

	if (matArch->gbufferFragBinary.size() == 0) {
		wip_CustomOverride = false;
		return;
	}

	vk::ShaderModuleCreateInfo createInfo{};
	createInfo.setCodeSize(matArch->gbufferFragBinary.size() * 4).setPCode(matArch->gbufferFragBinary.data());
	wip_New.fragModule = vl::Device->createShaderModuleUnique(createInfo);


	vk::ShaderModuleCreateInfo createInfo3{};
	createInfo3.setCodeSize(matArch->depthBinary.size() * 4).setPCode(matArch->depthBinary.data());
	wip_New.depthFragModule = vl::Device->createShaderModuleUnique(createInfo3);


	auto podPtr = podHandle.Lock();

	GpuAsset<Shader>& gbufferShader = GpuAssetManager->CompileShader("engine-data/spv/gbuffer.shader");

	vk::ShaderModule vert = gbufferShader.vert.Lock().module.get();

	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo
		.setStage(vk::ShaderStageFlagBits::eVertex) //
		.setModule(vert)
		.setPName("main");

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo
		.setStage(vk::ShaderStageFlagBits::eFragment) //
		.setModule(*wip_New.fragModule)
		.setPName("main");

	shaderStages.push_back(vertShaderStageInfo);
	shaderStages.push_back(fragShaderStageInfo);

	GpuAsset<Shader>& depthShader = GpuAssetManager->CompileShader("engine-data/spv/depth_map.shader");

	std::vector<vk::PipelineShaderStageCreateInfo> depthShaderStages;

	vk::PipelineShaderStageCreateInfo dvertShaderStageInfo{};
	dvertShaderStageInfo
		.setStage(vk::ShaderStageFlagBits::eVertex) //
		.setModule(depthShader.vert.Lock().module.get())
		.setPName("main");

	vk::PipelineShaderStageCreateInfo dfragShaderStageInfo{};
	dfragShaderStageInfo
		.setStage(vk::ShaderStageFlagBits::eFragment) //
		.setModule(*wip_New.depthFragModule)
		.setPName("main");

	depthShaderStages.push_back(dvertShaderStageInfo);
	depthShaderStages.push_back(dfragShaderStageInfo);


	{
		auto createDescLayout = [&]() {
			wip_New.descLayout = std::make_unique<RDescriptorLayout>();
			wip_New.descLayout->AddBinding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
			for (uint32 i = 0; i < matArch->descriptorSetLayout.samplers2d.size(); ++i) {
				wip_New.descLayout->AddBinding(
					vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
			}
			wip_New.descLayout->Generate();
		};

		if (!wip_New.descLayout) {
			createDescLayout();
		}
		else if (wip_New.descLayout->bindings.size() != matArch->descriptorSetLayout.samplers2d.size() + 1) {
			createDescLayout();
		}
	}

	{
		// pipeline layout
		vk::PushConstantRange pushConstantRange{};
		pushConstantRange
			.setStageFlags(vk::ShaderStageFlagBits::eVertex) //
			.setSize(sizeof(PC))
			.setOffset(0u);

		std::array layouts = { wip_New.descLayout->setLayout.get(), Layouts->singleUboDescLayout.setLayout.get() };

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo
			.setSetLayoutCount(static_cast<uint32>(layouts.size())) //
			.setPSetLayouts(layouts.data())
			.setPushConstantRangeCount(1u)
			.setPPushConstantRanges(&pushConstantRange);

		wip_New.plLayout = Device->createPipelineLayoutUnique(pipelineLayoutInfo);
	}

	{
		// depth pipeline layout
		vk::PushConstantRange pushConstantRange{};
		pushConstantRange
			.setStageFlags(vk::ShaderStageFlagBits::eVertex) //
			.setSize(sizeof(PushConstantShadow))
			.setOffset(0u);

		std::array layouts = { wip_New.descLayout->setLayout.get() };

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo
			.setSetLayoutCount(static_cast<uint32>(layouts.size())) //
			.setPSetLayouts(layouts.data())
			.setPushConstantRangeCount(1u)
			.setPPushConstantRanges(&pushConstantRange);

		wip_New.depthPlLayout = Device->createPipelineLayoutUnique(pipelineLayoutInfo);
	}

	{
		wip_New.pipeline = GBufferPass::CreatePipeline(*wip_New.plLayout, shaderStages);

		wip_New.depthPipeline = DepthmapPass::CreatePipeline(*wip_New.depthPlLayout, depthShaderStages);
	}

	{
		wip_New.descSet = wip_New.descLayout->GetDescriptorSet();
		if (!wip_New.uboBuf) {
			wip_New.uboBuf
				= std::make_unique<vl::RBuffer>(sizeof(UBO_Material) * 4, vk::BufferUsageFlagBits::eUniformBuffer,
					vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		}
		wip_New.uboBuf->UploadData(matInst->descriptorSet.uboData);

		vk::DescriptorBufferInfo bufferInfo{};
		bufferInfo
			.setBuffer(*wip_New.uboBuf) //
			.setOffset(0u)
			.setRange(matArch->descriptorSetLayout.SizeOfUbo());
		vk::WriteDescriptorSet descriptorWrite{};

		descriptorWrite
			.setDstSet(wip_New.descSet) //
			.setDstBinding(0u)
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1u)
			.setPBufferInfo(&bufferInfo)
			.setPImageInfo(nullptr)
			.setPTexelBufferView(nullptr);

		Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);

		auto UpdateImageSamplerInDescriptorSet = [&](vk::Sampler sampler, GpuHandle<Image> image, uint32 dstBinding) {
			auto& img = image.Lock();

			vk::DescriptorImageInfo imageInfo{};
			imageInfo
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
				.setImageView(img.image->GetView())
				.setSampler(sampler);

			vk::WriteDescriptorSet descriptorWrite{};
			descriptorWrite
				.setDstSet(wip_New.descSet) //
				.setDstBinding(dstBinding)
				.setDstArrayElement(0u)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setDescriptorCount(1u)
				.setPBufferInfo(nullptr)
				.setPImageInfo(&imageInfo)
				.setPTexelBufferView(nullptr);

			// PERF: Use a single descriptor update
			Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
		};


		for (uint32 i = 0; i < matInst->descriptorSet.samplers2d.size(); ++i) {
			UpdateImageSamplerInDescriptorSet(GpuAssetManager->GetDefaultSampler(),
				GpuAssetManager->GetGpuHandle(matInst->descriptorSet.samplers2d[i]), i + 1);
		}
	}
	wip_CustomOverride = true;
}
