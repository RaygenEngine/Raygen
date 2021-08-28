#pragma once
#include "rendering/pipes/StaticPipeBase.h"

struct SceneReflprobe;

namespace vl {

struct MomentsBufferCalculationPipe : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	// DOC: parameters
	void RecordCmd(vk::CommandBuffer cmdBuffer, const vk::Extent3D& extent, vk::DescriptorSet inputOutputsImageDescSet,
		const SceneRenderDesc& sceneDesc) const;
};
} // namespace vl
