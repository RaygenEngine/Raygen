#pragma once
#include "rendering/wrappers/DescriptorLayout.h"

namespace vl {
inline struct Layouts_ {

	RDescriptorLayout regularMaterialDescLayout;
	RDescriptorLayout gbufferDescLayout;
	RDescriptorLayout singleUboDescLayout;
	RDescriptorLayout jointsDescLayout;
	RDescriptorLayout singleSamplerDescLayout;
	RDescriptorLayout cubemapLayout;
	RDescriptorLayout envmapLayout;
	RDescriptorLayout accelLayout;


	RDescriptorLayout imageDebugDescLayout;

	vk::UniqueRenderPass depthRenderPass;
	vk::UniqueRenderPass gbufferPass;

	Layouts_();


} * Layouts{};
} // namespace vl
