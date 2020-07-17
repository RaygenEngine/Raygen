#pragma once
#include "assets/pods/EnvironmentMap.h"
#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/wrappers/RImageAttachment.h"

#include <vulkan/vulkan.hpp>
namespace vl {

class BrdfLutCalculation {

	vk::UniqueRenderPass m_renderPass;

	vk::CommandBuffer m_cmdBuffer;

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	vk::UniqueFramebuffer m_framebuffer;
	UniquePtr<RImageAttachment> m_attachment;

	EnvironmentMap::Gpu* m_envmapAsset;

	uint32 m_resolution;

	void MakeRenderPass();
	void AllocateCommandBuffers();
	void MakePipeline();
	void PrepareFaceInfo();
	void RecordAndSubmitCmdBuffers();
	void EditPods();

public:
	BrdfLutCalculation(EnvironmentMap::Gpu* envmapAsset, uint32 calculationResolution);

	void Calculate();
};
} // namespace vl
