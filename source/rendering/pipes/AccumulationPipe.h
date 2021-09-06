#pragma once
#include "rendering/pipes/StaticPipeBase.h"

namespace vl {

struct AccumulationPipe : public StaticPipeBase {

	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void RecordCmd(vk::CommandBuffer cmdBuffer, const vk::Extent3D& extent, vk::DescriptorSet inputOutputStorageImages,
		int32 iteration) const;
};
} // namespace vl
