#pragma once
#include "rendering/wrappers/DescriptorSetLayout.h"

namespace vl {
inline struct Layouts_ {

	RDescriptorSetLayout gltfMaterialDescLayout;
	RDescriptorSetLayout gbufferDescLayout;
	RDescriptorSetLayout singleUboDescLayout;
	RDescriptorSetLayout jointsDescLayout;
	RDescriptorSetLayout singleSamplerDescLayout;
	RDescriptorSetLayout cubemapLayout;
	RDescriptorSetLayout envmapLayout;
	RDescriptorSetLayout accelLayout;

	RDescriptorSetLayout rtTriangleGeometry;

	RDescriptorSetLayout singleStorageImage;

	RDescriptorSetLayout rtSceneDescLayout;


	RDescriptorSetLayout imageDebugDescLayout;

	vk::UniqueRenderPass depthRenderPass;
	vk::UniqueRenderPass gbufferPass;

	Layouts_();


} * Layouts{};
} // namespace vl
