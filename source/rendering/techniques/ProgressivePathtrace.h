#pragma once
#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/ImageView.h"

struct SceneRenderDesc;

namespace vl {

enum class PtMode
{
	Naive,
	Stochastic,
	Bdpt,
};

struct ProgressivePathtrace {
	ProgressivePathtrace();

	RImage2D pathtraced;
	vk::DescriptorSet pathtracedDescSet;
	RImage2D progressive;
	vk::DescriptorSet inputOutputDescSet;

	// use for compatibility with other callers of pathtracing
	RBuffer viewer;
	vk::DescriptorSet viewerDescSet;
	BoolFlag updateViewer{ true };

	int32 iteration{ 0 };

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, int32 samples, int32 bounces,
		PtMode mode = PtMode::Stochastic);
	void Resize(vk::Extent2D extent);
};

} // namespace vl
