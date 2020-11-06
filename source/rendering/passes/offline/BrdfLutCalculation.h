#pragma once
#include "rendering/wrappers/ImageView.h"


namespace vl {

class BrdfLutCalculation {

	vk::UniqueRenderPass m_renderPass;

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	vk::UniqueFramebuffer m_framebuffer;
	RImageAttachment m_attachment;

	GpuEnvironmentMap* m_envmapAsset;

	uint32 m_resolution;

	void MakeRenderPass();
	void MakePipeline();
	void PrepareFaceInfo();
	void RecordAndSubmitCmdBuffers();
	void EditPods();

public:
	BrdfLutCalculation(GpuEnvironmentMap* envmapAsset, uint32 calculationResolution);

	void Calculate();
};
} // namespace vl