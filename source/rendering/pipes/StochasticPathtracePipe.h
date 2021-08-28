#pragma once
#include "rendering/pipes/StaticPipeBase.h"

namespace vl {
struct StochasticPathtracePipe : public StaticRaytracingPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const vk::Extent3D& extent,
		vk::DescriptorSet storageImageDescSet, vk::DescriptorSet viewerDescSet, int32 seed, int32 samples,
		int32 bounces) const;
};
} // namespace vl
