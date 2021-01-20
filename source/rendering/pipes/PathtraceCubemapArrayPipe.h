#pragma once
#include "rendering/pipes/StaticPipeBase.h"

struct SceneIrragrid;

namespace vl {
struct PathtraceCubemapArrayPipe : public StaticRaytracingPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const SceneIrragrid& ig) const;
};
} // namespace vl
