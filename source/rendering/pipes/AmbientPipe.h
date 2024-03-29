#pragma once
#include "rendering/pipes/StaticPipeBase.h"

namespace vl {

struct AmbientPipe : public StaticPipeBase {
	vk::UniquePipelineLayout MakePipelineLayout() override;
	vk::UniquePipeline MakePipeline() override;

	void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, int32 samples, float bias,
		float strength, float radius) const;
};

} // namespace vl
