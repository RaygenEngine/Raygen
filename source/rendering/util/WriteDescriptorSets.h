#pragma once

namespace rvk {
// void writeDescriptorImages(vk::DescriptorSet descSet, uint32 firstBinding, std::vector<vk::ImageView>&& imageViews,
//	vk::Sampler sampler = vk::Sampler(nullptr));


void writeDescriptorImages(vk::DescriptorSet descSet, uint32 firstBinding, std::vector<vk::ImageView>&& imageViews,
	vk::DescriptorType descriptorType = vk::DescriptorType::eCombinedImageSampler,
	vk::Sampler sampler = vk::Sampler(nullptr), vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal);

} // namespace rvk
