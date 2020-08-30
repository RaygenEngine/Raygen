#pragma once
#include "rendering/wrappers/DescriptorSetLayout.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

namespace vl {

enum GColorAttachment : uint32
{
	GNormal = 0,
	// rgb: color, a: opacity
	GBaseColor = 1,
	// r: metallic, g: roughness, b: reflectance, a: occlusion
	GSurface = 2,
	GEmissive = 3,
	GDepth = 4,
	GCount
};

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
	RDescriptorSetLayout doubleStorageImage;

	RDescriptorSetLayout rtSceneDescLayout;


	RDescriptorSetLayout imageDebugDescLayout;

	vk::UniqueRenderPass depthRenderPass;
	vk::UniqueRenderPass lightblendPass;

	RRenderPassLayout ptPassLayout;
	RRenderPassLayout gbufferPassLayout;

	void MakeRenderPassLayouts();

	Layouts_();
} * Layouts{};
} // namespace vl
