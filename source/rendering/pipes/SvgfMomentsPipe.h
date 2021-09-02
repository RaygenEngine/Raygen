#pragma once
#include "rendering/pipes/StaticPipeBase.h"

struct SceneReflprobe;

namespace vl {

struct SvgfMomentsPipe : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	// DOC: parameters
	void RecordCmd(vk::CommandBuffer cmdBuffer, const vk::Extent3D& extent, const SceneRenderDesc& sceneDesc,
		vk::DescriptorSet inputOutputsImageDescSet, float minColorAlpha, float minMomentsAlpha,
		bool luminanceMode) const;
};
} // namespace vl
