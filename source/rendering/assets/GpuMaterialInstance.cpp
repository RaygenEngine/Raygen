#include "pch.h"
#include "GpuMaterialInstance.h"

#include "assets/pods/MaterialInstance.h"
#include "rendering/assets/GpuMaterialArchetype.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Renderer.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/passes/DepthmapPass.h"


using namespace vl;

MaterialInstance::Gpu::Gpu(PodHandle<MaterialInstance> podHandle)
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

void MaterialInstance::Gpu::Update(const AssetUpdateInfo& info)
{
	auto data = podHandle.Lock();
	ClearDependencies();
	AddDependency(data->archetype);

	archetype = GpuAssetManager->GetGpuHandle(data->archetype);

	auto matInst = podHandle.Lock();
	auto matArch = matInst->archetype.Lock();
	auto& gpuArch = GpuAssetManager->GetGpuHandle(matInst->archetype).Lock();

	gbufferPipeline = GbufferPass::CreatePipeline(*gpuArch.gbufferPipelineLayout, gpuArch.gbufferShaderStages);
	depthPipeline = DepthmapPass::CreatePipeline(*gpuArch.depthPipelineLayout, gpuArch.depthShaderStages);

	{
		if (gpuArch.descLayout->IsEmpty()) {
			hasDescriptorSet = false;
			return;
		}
		hasDescriptorSet = true;
		descSet = gpuArch.descLayout->GetDescriptorSet();


		if (gpuArch.descLayout->hasUbo) {
			if (!uboBuf) {
				// WIP: UBO SIZE
				// PERF: NEXT:
				uboBuf = std::make_unique<vl::RBuffer>(2048, vk::BufferUsageFlagBits::eUniformBuffer,
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
