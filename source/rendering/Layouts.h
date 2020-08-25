#pragma once
#include "rendering/wrappers/DescriptorLayout.h"

namespace vl {
inline struct Layouts_ {

	RDescriptorLayout gltfMaterialDescLayout;
	RDescriptorLayout gbufferDescLayout;
	RDescriptorLayout singleUboDescLayout;
	RDescriptorLayout jointsDescLayout;
	RDescriptorLayout singleSamplerDescLayout;
	RDescriptorLayout cubemapLayout;
	RDescriptorLayout envmapLayout;
	RDescriptorLayout accelLayout;

	RDescriptorLayout rtTriangleGeometry;

	RDescriptorLayout singleStorageImage;

	RDescriptorLayout rtSceneDescLayout;


	RDescriptorLayout imageDebugDescLayout;

	vk::UniqueRenderPass depthRenderPass;
	vk::UniqueRenderPass gbufferPass;
	vk::UniqueRenderPass lightblendPass;

	Layouts_();


} * Layouts{};
} // namespace vl
