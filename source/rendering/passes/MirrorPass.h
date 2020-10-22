#pragma once
#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/ImageView.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"
#include "rendering/ppt/PtBase.h"

struct SceneRenderDesc;

namespace vl {

class MirrorPass {
public:
	vk::UniquePipeline m_rtPipeline;
	vk::UniquePipelineLayout m_rtPipelineLayout;
	RBuffer m_rtSBTBuffer;
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_rtShaderGroups;

	InFlightResources<RImageAttachment> m_indirectResult;

	InFlightResources<vk::DescriptorSet> m_rtDescSet;

	void MakeRtPipeline();

	void CreateRtShaderBindingTable();

	void RecordPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);

	void Resize(vk::Extent2D extent);
};

} // namespace vl
