#include "GpuMaterialInstance.h"

#include "assets/StdAssets.h"
#include "assets/pods/MaterialArchetype.h"
#include "assets/pods/MaterialInstance.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/assets/GpuMaterialArchetype.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/passes/DepthmapPass.h"


using namespace vl;

GpuMaterialInstance::GpuMaterialInstance(PodHandle<MaterialInstance> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({}); // NOTE: Virtual function call in constructor will not call subclasses overrides, thats why we
				// explicitly mark this function as final in the header
}

void GpuMaterialInstance::Update(const AssetUpdateInfo& info)
{
	auto data = podHandle.Lock();
	ClearDependencies();
	AddDependency(data->archetype);

	archetype = GpuAssetManager->GetGpuHandle(data->archetype);

	auto matInst = podHandle.Lock();
	auto matArch = matInst->archetype.Lock();
	auto& gpuArch = GpuAssetManager->GetGpuHandle(matInst->archetype).Lock();


	{
		if (gpuArch.descLayout.IsEmpty()) {
			hasDescriptorSet = false;
			return;
		}
		hasDescriptorSet = true;
		descSet = gpuArch.descLayout.AllocDescriptorSet();


		size_t uboSize = matInst->descriptorSet.uboData.size();
		if (gpuArch.descLayout.HasUbo() && uboSize > 0) {

			if (uboSize != uboBuf.size) {
				uboBuf = RBuffer{ uboSize, vk::BufferUsageFlagBits::eUniformBuffer,
					vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
			}
			uboBuf.UploadData(matInst->descriptorSet.uboData);

			vk::DescriptorBufferInfo bufferInfo{};
			bufferInfo
				.setBuffer(uboBuf.handle()) //
				.setOffset(0u)
				.setRange(matArch->descriptorSetLayout.SizeOfUbo());
			vk::WriteDescriptorSet descriptorWrite{};

			descriptorWrite
				.setDstSet(descSet) //
				.setDstBinding(0u)
				.setDstArrayElement(0u)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setBufferInfo(bufferInfo);

			Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
		}

		int32 samplersBeginOffset = gpuArch.descLayout.HasUbo() && uboSize > 0 ? 1 : 0;

		auto UpdateImageSamplerInDescriptorSet = [&](vk::Sampler sampler, GpuHandle<Image> image, uint32 dstBinding) {
			auto& img = image.Lock();

			vk::DescriptorImageInfo imageInfo{};
			imageInfo
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
				.setImageView(img.image.view())
				.setSampler(sampler);

			vk::WriteDescriptorSet descriptorWrite{};
			descriptorWrite
				.setDstSet(descSet) //
				.setDstBinding(dstBinding)
				.setDstArrayElement(0u)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setImageInfo(imageInfo);

			// PERF: Use a single descriptor update
			Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
		};


		for (uint32 i = 0; i < matInst->descriptorSet.samplers2d.size(); ++i) {
			UpdateImageSamplerInDescriptorSet(GpuAssetManager->GetDefaultSampler(),
				GpuAssetManager->GetGpuHandle(matInst->descriptorSet.samplers2d[i]), i + samplersBeginOffset);
		}
	}

	UpdateRtMaterial(info);
}

void GpuMaterialInstance::UpdateRtMaterial(const AssetUpdateInfo& info)
{
	if (archetype != StdAssets::GltfArchetype()) {
		return; // NEXT:
	}
	auto& samplers = podHandle.Lock()->descriptorSet.samplers2d;

	struct RtGltfMat {
		// factors
		glm::vec4 baseColorFactor;
		glm::vec4 emissiveFactor;
		float metallicFactor;
		float roughnessFactor;
		float normalScale;
		float occlusionStrength;

		// alpha mask
		float alphaCutoff;
		int mask;

		uint32 baseColor;
		uint32 metallicRough;
		uint32 occlusion;
		uint32 normal;
		uint32 emissive;
	};
	auto matInst = podHandle.Lock();

	rtMaterialBuffer = RBuffer{ sizeof(RtGltfMat),
		vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		vk::MemoryAllocateFlagBits::eDeviceAddress };


	rtMaterialBuffer.UploadData(matInst->descriptorSet.uboData);
	// rtMaterialBuffer.CopyBuffer(uboBuf);


	size_t cursor = offsetof(RtGltfMat, baseColor);
	uint32 tempIndex = 0;
	std::vector<uint32> imageIndexes;

	for (auto& sampler : samplers) {
		imageIndexes.emplace_back(static_cast<uint32>(sampler.uid));
	}

	rtMaterialBuffer.UploadData(imageIndexes.data(), sizeof(uint32) * 5, cursor);
}
