#pragma once
#include "rendering/wrappers/RDescriptorLayout.h"

#include <vulkan/vulkan.hpp>

namespace vl {
inline struct Layouts_ {

	RDescriptorLayout regularMaterialDescLayout;
	RDescriptorLayout gbufferDescLayout;
	RDescriptorLayout singleUboDescLayout;
	RDescriptorLayout jointsDescLayout;
	RDescriptorLayout singleSamplerDescLayout;
	RDescriptorLayout cubemapLayout;
	RDescriptorLayout envmapLayout;


	RDescriptorLayout imageDebugDescLayout;

	vk::UniqueRenderPass depthRenderPass;
	vk::UniqueRenderPass gbufferPass;

	Layouts_();


} * Layouts{};
} // namespace vl
