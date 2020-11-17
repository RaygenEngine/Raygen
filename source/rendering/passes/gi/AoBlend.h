#pragma once
#include "rendering/StaticPipeBase.h"

namespace vl {

struct AoBlend : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, vk::DescriptorSet inputDescSet) const;
};

} // namespace vl
