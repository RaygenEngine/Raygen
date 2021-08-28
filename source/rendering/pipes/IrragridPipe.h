#pragma once
#include "rendering/pipes/StaticPipeBase.h"

namespace vl {

struct IrragridPipe : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	// DOC: parameters
	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, vk::DescriptorSet inputDescSet) const;
};

} // namespace vl
