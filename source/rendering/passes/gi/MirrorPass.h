#pragma once
#include "rendering/ppt/PtBase.h"
#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/ImageView.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

struct SceneRenderDesc;

namespace vl {

class MirrorPass {
public:
	MirrorPass();

	void RecordPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);

	void Resize(vk::Extent2D extent);

	InFlightResources<RImageAttachment> m_result;

	void MakeRtPipeline();
	void CreateRtShaderBindingTable();

private:
	vk::UniquePipeline m_rtPipeline;
	vk::UniquePipelineLayout m_rtPipelineLayout;

	RBuffer m_rtSBTBuffer;
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_rtShaderGroups;
	InFlightResources<vk::DescriptorSet> m_rtDescSet;
};
} // namespace vl
