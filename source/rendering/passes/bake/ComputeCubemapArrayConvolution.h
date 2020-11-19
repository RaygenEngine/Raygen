#pragma once
#include "rendering/wrappers/Buffer.h"

struct SceneRenderDesc;
struct SceneIrradianceGrid;

namespace vl {

class ComputeCubemapArrayConvolution {
public:
	ComputeCubemapArrayConvolution();

	void RecordPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const SceneIrradianceGrid& ig);

private:
	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	void MakeCompPipeline();
};
} // namespace vl
