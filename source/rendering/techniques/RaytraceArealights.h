#pragma once
#include "rendering/wrappers/ImageView.h"

struct SceneRenderDesc;

namespace vl {
struct RaytraceArealights {

	RaytraceArealights();

	RImage2D progressive;
	InFlightResources<RImage2D> result;
	InFlightResources<vk::DescriptorSet> descSet;

	int32 frame{ 0 };

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
	void Resize(vk::Extent2D extent);
};

} // namespace vl
