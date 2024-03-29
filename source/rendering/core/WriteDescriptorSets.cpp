#include "WriteDescriptorSets.h"

#include "rendering/Device.h"
#include "rendering/assets/GpuAssetManager.h"

using namespace vl;

//
void rvk::writeDescriptorImages(vk::DescriptorSet descSet, uint32 firstBinding, std::vector<vk::ImageView>&& imageViews,
	vk::DescriptorType descriptorType, vk::ImageLayout layout, vk::Sampler sampler)
{
	if (!sampler) {
		sampler = GpuAssetManager->GetDefaultSampler();
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

	Device->updateDescriptorSets(descWrites, nullptr);
}

void rvk::writeDescriptorImages(vk::DescriptorSet descSet, uint32 firstBinding,
	std::vector<ImageSampler>&& imageSamplers, vk::DescriptorType descriptorType, vk::ImageLayout layout)
{
	CLOG_ABORT(descriptorType != vk::DescriptorType::eCombinedImageSampler
				   && descriptorType != vk::DescriptorType::eSampledImage,
		"Invalid Descriptor Type for this helper function");


	std::vector<vk::DescriptorImageInfo> imageInfos;
	imageInfos.reserve(imageSamplers.size());

	for (auto& imageSampler : imageSamplers) {
		auto& info = imageInfos.emplace_back();
		info //
			.setImageLayout(layout)
			.setImageView(imageSampler.imageView)
			.setSampler(!imageSampler.sampler ? GpuAssetManager->GetDefaultSampler() : imageSampler.sampler);
	}

	std::vector<vk::WriteDescriptorSet> descWrites;
	descWrites.reserve(imageSamplers.size());

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

	Device->updateDescriptorSets(descWrites, nullptr);
}

void rvk::writeDescriptorImageArray(vk::DescriptorSet descSet, uint32 targetBinding,
	std::vector<vk::ImageView>&& imageViews, vk::Sampler sampler, vk::DescriptorType descriptorType,
	vk::ImageLayout layout)
{
	if (!sampler) {
		sampler = GpuAssetManager->GetDefaultSampler();
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

	// NOTE: this has to be a seperate loop to not invalidate PImageInfo parameter
	vk::WriteDescriptorSet descriptorWrite{};
	descriptorWrite
		.setDstSet(descSet) //
		.setDstBinding(targetBinding)
		.setDstArrayElement(0u)
		.setDescriptorType(descriptorType)
		.setImageInfo(imageInfos);


	Device->updateDescriptorSets(descriptorWrite, nullptr);
}

void rvk::writeDescriptorImages(vk::DescriptorSet descSet, uint32 firstBinding, std::vector<vk::ImageView>&& imageViews,
	vk::Sampler sampler, vk::DescriptorType descriptorType, vk::ImageLayout layout)
{
	rvk::writeDescriptorImages(descSet, firstBinding, std::move(imageViews), descriptorType, layout, sampler);
}

void rvk::writeDescriptorBuffer(
	vk::DescriptorSet descSet, uint32 targetBinding, vk::Buffer buffer, size_t size, vk::DescriptorType descriptorType)
{
	vk::DescriptorBufferInfo bufferInfo{};

	bufferInfo
		.setBuffer(buffer) //
		.setOffset(0u)
		.setRange(size);
	vk::WriteDescriptorSet descriptorWrite{};

	descriptorWrite
		.setDstSet(descSet) //
		.setDstBinding(targetBinding)
		.setDstArrayElement(0u)
		.setDescriptorType(descriptorType)
		.setBufferInfo(bufferInfo);

	Device->updateDescriptorSets(descriptorWrite, nullptr);
}
