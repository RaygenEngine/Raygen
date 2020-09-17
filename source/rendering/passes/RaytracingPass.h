#pragma once
#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/ImageView.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"
#include "rendering/ppt/PtBase.h"

struct SceneRenderDesc;

namespace vl {
class Renderer_;

class RaytracingPass {
public:
	vk::UniquePipeline m_rtPipeline;
	vk::UniquePipelineLayout m_rtPipelineLayout;
	RBuffer m_rtSBTBuffer;
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_rtShaderGroups;

	RImage2D m_progressiveResult;

	InFlightResources<RImageAttachment> m_indirectResult;


	int32 m_rtFrame{ 0 };

	void MakeRtPipeline();

	void CreateRtShaderBindingTable();

	void RecordPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, Renderer_* renderer);

	void Resize(vk::Extent2D extent);
};

} // namespace vl
