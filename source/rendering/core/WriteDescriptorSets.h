#pragma once

#include <vulkan/vulkan.hpp>

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

struct ImageSampler{
	vk::ImageView imageView; // keep order for clean emplace init
	vk::Sampler sampler;
};

void writeDescriptorImages(
	vk::DescriptorSet descSet,
	uint32 firstBinding,
	std::vector<ImageSampler>&& imageSamplers,
	vk::DescriptorType descriptorType = vk::DescriptorType::eCombinedImageSampler,
	vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal
);

void writeDescriptorImageArray(vk::DescriptorSet descSet, uint32 targetBinding, std::vector<vk::ImageView>&& imageViews,
	vk::Sampler sampler = vk::Sampler(nullptr),
	vk::DescriptorType descriptorType = vk::DescriptorType::eCombinedImageSampler,
	vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal);

void writeDescriptorBuffer(vk::DescriptorSet descSet, uint32 targetBinding, vk::Buffer buffer, size_t size = VK_WHOLE_SIZE, vk::DescriptorType descriptorType = vk::DescriptorType::eUniformBuffer);


} // namespace rvk
