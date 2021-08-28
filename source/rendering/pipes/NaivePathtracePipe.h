#pragma once
#include "rendering/pipes/StaticPipeBase.h"
#include "rendering/wrappers/Buffer.h"

namespace vl {
struct NaivePathtracePipe : public StaticRaytracingPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	// DOC: parameters
	void RecordCmd(vk::CommandBuffer cmdBuffer, const vk::Extent3D& extent, const SceneRenderDesc& sceneDesc,
		vk::DescriptorSet storageImageDescSet, vk::DescriptorSet viewerDescSet, int32 seed, int32 samples,
		int32 bounces) const;
};
} // namespace vl
