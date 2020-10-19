#pragma once
#include "rendering/wrappers/DescriptorSetLayout.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

namespace vl {

// NEXT:
enum GColorAttachment : uint32
{
	GDepth = 0,
	GNormal = 1,
	GAlbedo = 2,
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


	RDescriptorSetLayout singleStorageImage = GenerateStorageImageDescSet(1);
	RDescriptorSetLayout doubleStorageImage = GenerateStorageImageDescSet(2);
	RDescriptorSetLayout tripleStorageImage = GenerateStorageImageDescSet(3);
	RDescriptorSetLayout quadStorageImage = GenerateStorageImageDescSet(4);

	RDescriptorSetLayout stbuffer;

	RDescriptorSetLayout bufferAndSamplersDescLayout;


	RDescriptorSetLayout imageDebugDescLayout;

	// Global descriptor Set
	RDescriptorSetLayout renderAttachmentsLayout;

	vk::UniqueRenderPass depthRenderPass;


	RRenderPassLayout gbufferPassLayout;
	RRenderPassLayout rasterDirectPassLayout; // PERF: subpass of gbuffer


	RRenderPassLayout svgfPassLayout;
	// Ray Trace Here
	RRenderPassLayout ptPassLayout;
	// Output pass


	void MakeRenderPassLayouts();

	Layouts_();

private:
	static RDescriptorSetLayout GenerateStorageImageDescSet(size_t Count);
} * Layouts{};
} // namespace vl
