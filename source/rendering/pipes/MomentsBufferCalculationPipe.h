#pragma once
#include "rendering/pipes/StaticPipeBase.h"

struct SceneReflprobe;

namespace vl {

struct MomentsBufferCalculationPipe : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	// DOC: parameters
	void RecordCmd(vk::CommandBuffer cmdBuffer, const vk::Extent3D& extent, const SceneRenderDesc& sceneDesc,
		vk::DescriptorSet inputOutputsImageDescSet, bool firstIter) const;
};
} // namespace vl
