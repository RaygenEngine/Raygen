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
	Update({}); // NOTE: Virtual function call in constructor will not call subclasses overrides, thats why we
				// explicitly mark this function as final in the header
}

struct PC {
	glm::mat4 modelMat;
	glm::mat4 normalMat;
};

struct PushConstantShadow {
	glm::mat4 mvp;
};

void Material::Gpu::Update(const AssetUpdateInfo& info)
{
	auto data = podHandle.Lock();

	ClearDependencies();
	AddDependency(data->wip_InstanceOverride);
	auto matInst = data->wip_InstanceOverride.Lock();
	auto matArch = matInst->archetype.Lock();


	vk::ShaderModuleCreateInfo createInfo{};
	createInfo.setCodeSize(matArch->gbufferFragBinary.size() * 4).setPCode(matArch->gbufferFragBinary.data());
	fragModule = vl::Device->createShaderModuleUnique(createInfo);


	vk::ShaderModuleCreateInfo createInfo3{};
	createInfo3.setCodeSize(matArch->depthBinary.size() * 4).setPCode(matArch->depthBinary.data());
	depthFragModule = vl::Device->createShaderModuleUnique(createInfo3);

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
		.setModule(*fragModule)
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
		.setModule(*depthFragModule)
		.setPName("main");

	depthShaderStages.push_back(dvertShaderStageInfo);
	depthShaderStages.push_back(dfragShaderStageInfo);


	{
		auto createDescLayout = [&]() {
			descLayout = std::make_unique<RDescriptorLayout>();
			if (matArch->descriptorSetLayout.SizeOfUbo() != 0) {
				descLayout->AddBinding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
			}

			for (uint32 i = 0; i < matArch->descriptorSetLayout.samplers2d.size(); ++i) {
				descLayout->AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
			}
			descLayout->Generate();
		};

		if (!descLayout) {
			createDescLayout();
		}
		else if (descLayout->bindings.size() != matArch->descriptorSetLayout.samplers2d.size() + 1) {
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

		std::array layouts = { descLayout->setLayout.get(), Layouts->singleUboDescLayout.setLayout.get() };

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo
			.setSetLayoutCount(static_cast<uint32>(layouts.size())) //
			.setPSetLayouts(layouts.data())
			.setPushConstantRangeCount(1u)
			.setPPushConstantRanges(&pushConstantRange);

		plLayout = Device->createPipelineLayoutUnique(pipelineLayoutInfo);
	}

	{
		// depth pipeline layout
		vk::PushConstantRange pushConstantRange{};
		pushConstantRange
			.setStageFlags(vk::ShaderStageFlagBits::eVertex) //
			.setSize(sizeof(PushConstantShadow))
			.setOffset(0u);

		std::array layouts = { descLayout->setLayout.get() };

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo
			.setSetLayoutCount(static_cast<uint32>(layouts.size())) //
			.setPSetLayouts(layouts.data())
			.setPushConstantRangeCount(1u)
			.setPPushConstantRanges(&pushConstantRange);

		depthPlLayout = Device->createPipelineLayoutUnique(pipelineLayoutInfo);
	}

	{
		pipeline = GbufferPass::CreatePipeline(*plLayout, shaderStages);

		depthPipeline = DepthmapPass::CreatePipeline(*depthPlLayout, depthShaderStages);
	}

	{
		if (descLayout->IsEmpty()) {
			hasDescriptorSet = false;
			return;
		}
		hasDescriptorSet = true;
		descSet = descLayout->GetDescriptorSet();


		if (descLayout->hasUbo) {
			if (!uboBuf) {
				// WIP:
				uboBuf
					= std::make_unique<vl::RBuffer>(sizeof(UBO_Material) * 4, vk::BufferUsageFlagBits::eUniformBuffer,
						vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			}
			uboBuf->UploadData(matInst->descriptorSet.uboData);

			vk::DescriptorBufferInfo bufferInfo{};
			bufferInfo
				.setBuffer(*uboBuf) //
				.setOffset(0u)
				.setRange(matArch->descriptorSetLayout.SizeOfUbo());
			vk::WriteDescriptorSet descriptorWrite{};

			descriptorWrite
				.setDstSet(descSet) //
				.setDstBinding(0u)
				.setDstArrayElement(0u)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1u)
				.setPBufferInfo(&bufferInfo)
				.setPImageInfo(nullptr)
				.setPTexelBufferView(nullptr);

			Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
		}

		auto UpdateImageSamplerInDescriptorSet = [&](vk::Sampler sampler, GpuHandle<Image> image, uint32 dstBinding) {
			auto& img = image.Lock();

			vk::DescriptorImageInfo imageInfo{};
			imageInfo
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
				.setImageView(img.image->GetView())
				.setSampler(sampler);

			vk::WriteDescriptorSet descriptorWrite{};
			descriptorWrite
				.setDstSet(descSet) //
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
}
