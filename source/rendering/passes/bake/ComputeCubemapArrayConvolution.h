#pragma once
#include "rendering/wrappers/Buffer.h"

struct SceneRenderDesc;
struct SceneIrragrid;

namespace vl {

class ComputeCubemapArrayConvolution {
public:
	ComputeCubemapArrayConvolution();

	void RecordPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const SceneIrragrid& ig);

private:
	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	void MakeCompPipeline();
};
} // namespace vl
