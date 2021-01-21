#pragma once

namespace rvk {
// clang-format off

// Same sampler for all images. Passing sampler if image is not sampled will assert
void writeDescriptorImages(
	vk::DescriptorSet descSet, 
	uint32 firstBinding, 
	std::vector<vk::ImageView>&& imageViews,
	vk::DescriptorType descriptorType = vk::DescriptorType::eCombinedImageSampler,
	vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal, 
	vk::Sampler sampler = vk::Sampler(nullptr)
);


// Sampler first version, same sampler for all images
void writeDescriptorImages(
	vk::DescriptorSet descSet, 
	uint32 firstBinding, 
	std::vector<vk::ImageView>&& imageViews,
	vk::Sampler sampler,
	vk::DescriptorType descriptorType = vk::DescriptorType::eCombinedImageSampler,
	vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal
);


void writeDescriptorImageArray(vk::DescriptorSet descSet, uint32 targetBinding, std::vector<vk::ImageView>&& imageViews,
	vk::Sampler sampler = vk::Sampler(nullptr),
	vk::DescriptorType descriptorType = vk::DescriptorType::eCombinedImageSampler,
	vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal);


} // namespace rvk
