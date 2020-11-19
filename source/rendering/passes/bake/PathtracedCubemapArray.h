#pragma once
#include "rendering/wrappers/Buffer.h"

struct SceneRenderDesc;
struct SceneIrradianceGrid;

namespace vl {

class PathtracedCubemapArray {
public:
	PathtracedCubemapArray();

	void RecordPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const SceneIrradianceGrid& ig);

private:
	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	RBuffer m_rtSBTBuffer;
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_rtShaderGroups;

	void MakeRtPipeline();
	void CreateRtShaderBindingTable();
};
} // namespace vl
