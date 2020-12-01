#pragma once
#include "rendering/StaticPipeBase.h"

namespace vl {
struct DirlightBlend : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const;
};

} // namespace vl