#pragma once
#include "rendering/pipes/StaticPipeBase.h"

namespace vl {

struct SvgfModulatePipe : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, vk::DescriptorSet inputDescSet) const;
};

} // namespace vl
