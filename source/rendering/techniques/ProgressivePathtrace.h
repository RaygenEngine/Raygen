#pragma once
#include "rendering/wrappers/ImageView.h"

struct SceneRenderDesc;

namespace vl {
struct ProgressivePathtrace {

	ProgressivePathtrace();

	RImage2D progressive;
	InFlightResources<RImage2D> result;
	InFlightResources<vk::DescriptorSet> descSet;

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, int32 frame);
	void Resize(vk::Extent2D extent);
};

} // namespace vl
