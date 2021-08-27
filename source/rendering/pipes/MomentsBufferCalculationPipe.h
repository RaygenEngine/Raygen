#pragma once
#include "rendering/pipes/StaticPipeBase.h"

struct SceneReflprobe;

namespace vl {

struct MomentsBufferCalculationPipe : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void Draw(vk::CommandBuffer cmdBuffer, vk::DescriptorSet inputOutputsImageDescSet, const SceneRenderDesc& sceneDesc,
		const vk::Extent3D& extent) const;
};
} // namespace vl
