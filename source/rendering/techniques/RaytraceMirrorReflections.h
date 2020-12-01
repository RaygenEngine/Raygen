#pragma once

struct SceneRenderDesc;

namespace vl {
struct RaytraceMirrorReflections {

	RaytraceMirrorReflections();

	InFlightResources<RImage2D> result;
	InFlightResources<vk::DescriptorSet> descSet;

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const;
	void Resize(vk::Extent2D extent) const;
};

} // namespace vl
