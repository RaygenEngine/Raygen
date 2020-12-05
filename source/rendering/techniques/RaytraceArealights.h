#pragma once
#include "rendering/wrappers/ImageView.h"

struct SceneRenderDesc;

namespace vl {
struct RaytraceArealights {

	RaytraceArealights();

	InFlightResources<RImage2D> result;
	InFlightResources<vk::DescriptorSet> descSet;

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const;
	void Resize(vk::Extent2D extent);
};

} // namespace vl
