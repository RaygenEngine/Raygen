#pragma once
#include "rendering/pipes/StaticPipeBase.h"

struct SceneIrragrid;

namespace vl {
struct CubemapArrayConvolutionPipe : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const SceneIrragrid& ig) const;
};
} // namespace vl
