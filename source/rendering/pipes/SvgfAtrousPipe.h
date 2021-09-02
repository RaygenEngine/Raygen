#pragma once
#include "rendering/pipes/StaticPipeBase.h"

namespace vl {

struct SvgfAtrousPipe : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc,
		vk::DescriptorSet inputOutputStorageImages, int32 iteration, int32 totalIter, bool luminanceMode) const;
};

} // namespace vl
