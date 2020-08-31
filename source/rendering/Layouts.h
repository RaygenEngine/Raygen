#pragma once
#include "rendering/wrappers/DescriptorSetLayout.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

namespace vl {

enum GColorAttachment : uint32
{
	GDepth = 0,
	GNormal = 1,
	// rgb: color, a: opacity
	GBaseColor = 2,
	// r: metallic, g: roughness, b: reflectance, a: occlusion
	GSurface = 3,
	GEmissive = 4,
	GCount
};

inline struct Layouts_ {

	RDescriptorSetLayout gltfMaterialDescLayout;
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

	RDescriptorSetLayout renderAttachmentsLayout;


	RRenderPassLayout gbufferPassLayout;
	RRenderPassLayout rasterDirectPassLayout; // TODO: subpass of gbuffer
	// Ray Trace Here
	RRenderPassLayout ptPassLayout;
	// Output pass


	void MakeRenderPassLayouts();

	Layouts_();
} * Layouts{};
} // namespace vl
