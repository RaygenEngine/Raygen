#include "pch.h"
#include "Layouts.h"

#include <glfw/glfw3.h>

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
	cameraDescLayout.AddBinding(
		vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment);
	cameraDescLayout.Generate();

	// spotlights
	spotlightDescLayout.AddBinding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
	spotlightDescLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	spotlightDescLayout.Generate();

	// ambient (TODO: reflection probes etc
	ambientDescLayout.AddBinding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
	ambientDescLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	ambientDescLayout.Generate();
}
} // namespace vl
