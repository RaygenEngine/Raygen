#pragma once
#include "rendering/wrappers/Buffer.h"

struct SceneRenderDesc;
struct SceneReflprobe;

namespace vl {

class ComputePrefilteredConvolution {
public:
	ComputePrefilteredConvolution();

	void RecordPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const SceneReflprobe& ig);

private:
	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	void MakeCompPipeline();
};
} // namespace vl
