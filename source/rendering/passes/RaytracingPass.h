#pragma once

#include "rendering/wrappers/Buffer.h"

struct SceneRenderDesc;

namespace vl {
class Renderer_;

class RaytracingPass {
public:
	vk::UniquePipeline m_rtPipeline;
	vk::UniquePipelineLayout m_rtPipelineLayout;
	RBuffer m_rtSBTBuffer;
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_rtShaderGroups;

	int32 m_rtFrame{ 0 };

	void MakeRtPipeline();

	void CreateRtShaderBindingTable();

	void RecordPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, Renderer_* renderer);
};

} // namespace vl
