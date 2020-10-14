#include "WriteDescriptorSets.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"

void rvk::writeDescriptorImages(vk::DescriptorSet descSet, uint32 firstBinding, std::vector<vk::ImageView>&& imageViews,
	vk::DescriptorType descriptorType, vk::Sampler sampler, vk::ImageLayout layout)
{
	if (!sampler) {
		sampler = vl::GpuAssetManager->GetDefaultSampler();
	}

	std::vector<vk::DescriptorImageInfo> imageInfos;
	imageInfos.reserve(imageViews.size());

	for (auto& view : imageViews) {
		auto& info = imageInfos.emplace_back();
		info //
			.setImageLayout(layout)
			.setImageView(view);

		if (descriptorType == vk::DescriptorType::eCombinedImageSampler
			|| descriptorType == vk::DescriptorType::eSampledImage) {
			info.setSampler(sampler);
		}
	}

	std::vector<vk::WriteDescriptorSet> descWrites;
	descWrites.reserve(imageViews.size());

	// NOTE: this has to be a seperate loop to not invalidate PImageInfo parameter
	for (uint32 i = 0; auto& info : imageInfos) {
		auto& descriptorWrite = descWrites.emplace_back();
		descriptorWrite
			.setDstSet(descSet) //
			.setDstBinding(i + firstBinding)
			.setDstArrayElement(0u)
			.setDescriptorType(descriptorType)
			.setDescriptorCount(1u)
			.setPImageInfo(&info);

		++i;
	}

	vl::Device->updateDescriptorSets(descWrites, nullptr);
}
