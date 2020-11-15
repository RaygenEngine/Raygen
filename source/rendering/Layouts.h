#pragma once
#include "rendering/wrappers/DescriptorSetLayout.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

namespace vl {

inline struct Layouts_ {

	RDescriptorSetLayout gltfMaterialDescLayout;
	RDescriptorSetLayout singleUboDescLayout;
	RDescriptorSetLayout jointsDescLayout;
	RDescriptorSetLayout singleSamplerDescLayout;
	RDescriptorSetLayout cubemapLayout;
	RDescriptorSetLayout envmapLayout;
	RDescriptorSetLayout accelLayout;

	RDescriptorSetLayout rtTriangleGeometry;

	RDescriptorSetLayout cubemapArray6;
	RDescriptorSetLayout cubemapArray64;
	RDescriptorSetLayout cubemapArray1024;
	RDescriptorSetLayout dynamicSamplerArray;

	RDescriptorSetLayout storageImageArray6;

	RDescriptorSetLayout singleStorageImage = GenerateStorageImageDescSet(1);
	RDescriptorSetLayout doubleStorageImage = GenerateStorageImageDescSet(2);
	RDescriptorSetLayout tripleStorageImage = GenerateStorageImageDescSet(3);
	RDescriptorSetLayout quadStorageImage = GenerateStorageImageDescSet(4);

	RDescriptorSetLayout singleStorageBuffer;

	RDescriptorSetLayout bufferAndSamplersDescLayout;


	RDescriptorSetLayout imageDebugDescLayout;

	// Global descriptor Set
	RDescriptorSetLayout renderAttachmentsLayout;

	RRenderPassLayout gbufferPassLayout;
	RRenderPassLayout shadowPassLayout;
	RRenderPassLayout singleFloatColorAttPassLayout;

	// CHECK: check if those can be only one - tho some techniques will require this split for sure
	RRenderPassLayout directLightPassLayout;   // PERF: subpass of gbuffer
	RRenderPassLayout indirectLightPassLayout; // PERF: subpass of gbuffer

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
