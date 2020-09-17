#pragma once
#include "rendering/wrappers/DescriptorSetLayout.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

namespace vl {

// NEXT:
enum GColorAttachment : uint32
{
	GDepth = 0,
	GNormal = 1,
	GDiffuseColor = 2,
	GSpecularColor = 3,
	GEmissive = 4,
	GVelocity = 5,
	GUVDrawIndex = 6,
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
	RDescriptorSetLayout tripleStorageImage;
	RDescriptorSetLayout quadStorageImage;


	RDescriptorSetLayout bufferAndSamplersDescLayout;


	RDescriptorSetLayout imageDebugDescLayout;

	// Global descriptor Set
	RDescriptorSetLayout renderAttachmentsLayout;

	vk::UniqueRenderPass depthRenderPass;


	RRenderPassLayout gbufferPassLayout;
	RRenderPassLayout rasterDirectPassLayout; // TODO: subpass of gbuffer


	RRenderPassLayout svgfPassLayout;
	// Ray Trace Here
	RRenderPassLayout ptPassLayout;
	// Output pass


	void MakeRenderPassLayouts();

	Layouts_();
} * Layouts{};
} // namespace vl
