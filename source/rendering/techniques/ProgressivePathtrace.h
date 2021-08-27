#pragma once
#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/ImageView.h"

struct SceneRenderDesc;

namespace vl {
struct ProgressivePathtrace {
	ProgressivePathtrace();

	RImage2D pathtraced;
	vk::DescriptorSet pathtracedDescSet;
	RImage2D progressive;
	vk::DescriptorSet inputOutputDescSet;

	// use for compatibility with other callers of pathtracing
	RBuffer viewer;
	vk::DescriptorSet viewerDescSet;

	int32 iteration{ 0 };

	BoolFlag updateViewer{ true };

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void Resize(vk::Extent2D extent);
};

} // namespace vl
