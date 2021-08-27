#pragma once

#include "rendering/techniques/ProgressivePathtrace.h"

struct SceneRenderDesc;

namespace vl {
struct TestSVGFProgPT {
	TestSVGFProgPT();

	RImage2D pathtraced;
	vk::DescriptorSet pathtracedDescSet;

	RImage2D progressiveVariance;
	RImage2D momentsHistory;
	vk::DescriptorSet inputOutputsDescSet;

	// use for compatibility with other callers of pathtracing
	RBuffer viewer;
	vk::DescriptorSet viewerDescSet;
	BoolFlag updateViewer{ true };

	int32 iteration{};

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, int32 samples, int32 bounces);
	void Resize(vk::Extent2D extent);
};

} // namespace vl
