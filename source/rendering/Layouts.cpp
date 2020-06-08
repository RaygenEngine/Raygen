#include "pch.h"
#include "Layouts.h"

namespace vl {
Layouts_::Layouts_()
{
	// gbuffer
	for (uint32 i = 0u; i < 6u; ++i) {
		gBufferDescLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	}
	gBufferDescLayout.Generate();

	// reg mat
	regularMaterialDescLayout.AddBinding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
	for (uint32 i = 0; i < 5u; ++i) {
		regularMaterialDescLayout.AddBinding(
			vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	}
	regularMaterialDescLayout.Generate();

	// camera
	// WIP/PERF: could have two seperate for each stage
	singleUboDescLayout.AddBinding(
		vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
	singleUboDescLayout.Generate();

	// spotlights
	singleSamplerDescLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	singleSamplerDescLayout.Generate();

	// cubemap
	cubemapLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	cubemapLayout.Generate();

	// evnmap
	envmapLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	envmapLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	envmapLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	envmapLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	envmapLayout.Generate();
}
} // namespace vl
