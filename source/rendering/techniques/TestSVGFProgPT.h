#pragma once

#include "rendering/techniques/ProgressivePathtrace.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

struct SceneRenderDesc;

namespace vl {
struct TestSVGFProgPT {
	TestSVGFProgPT();

	RImage2D pathtracedResult;
	vk::DescriptorSet pathtracingInputDescSet;

	RImage2D progressive;

	RImage2D momentsHistory;
	vk::DescriptorSet inputOutputsDescSet;

	// use for compatibility with other callers of pathtracing
	RBuffer viewer;
	vk::DescriptorSet viewerDescSet;
	BoolFlag updateViewer{ true };

	InFlightResources<RenderingPassInstance> svgfRenderPassInstance;

	// DescSet 0:
	//    Img 0 is read  (index: 2)
	//    Img 1 is write (index: 3)
	//
	// DescSet 1:
	//    Img 1 is read  (index: 2)
	//    Img 0 is write (index: 3)
	std::array<vk::DescriptorSet, 2> descriptorSets;
	std::array<RImage2D, 2> swappingImages;


	int32 iteration{};

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, int32 samples, int32 bounces);
	void Resize(vk::Extent2D extent);
};

} // namespace vl
