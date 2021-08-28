#pragma once
#include "rendering/pipes/StaticPipeBase.h"

namespace vl {
struct ArealightsPipe : public StaticRaytracingPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	// DOC: parameters
	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const vk::Extent3D& extent,
		vk::DescriptorSet storageImagesDescSet, int32 frame) const;
};
} // namespace vl
