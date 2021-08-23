#pragma once
#include "rendering/pipes/StaticPipeBase.h"

namespace vl {
struct TestSptPipe : public StaticRaytracingPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, vk::DescriptorSet storageImagesDescSet,
		const vk::Extent3D& extent, int32 iteration, int32 samples, int32 bounces) const;
};
} // namespace vl
