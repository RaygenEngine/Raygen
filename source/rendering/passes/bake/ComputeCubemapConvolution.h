#pragma once
#include "rendering/wrappers/Buffer.h"

struct SceneRenderDesc;
struct SceneReflprobe;

namespace vl {

class ComputeCubemapConvolution {
public:
	ComputeCubemapConvolution();

	void RecordPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const SceneReflprobe& ig);

private:
	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	void MakeCompPipeline();
};
} // namespace vl
