#pragma once
#include "rendering/pipes/StaticPipeBase.h"

struct SceneReflprobe;

namespace vl {

struct AccumulationPipe : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void Draw(vk::CommandBuffer cmdBuffer, vk::DescriptorSet inputOutputStorageImages, const vk::Extent3D& extent,
		int32 iteration) const;
};
} // namespace vl
