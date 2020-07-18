#pragma once
#include "rendering/wrappers/RDescriptorLayout.h"
#include "rendering/wrappers/RImageAttachment.h"

namespace vl {
class RBuffer;

// CHECK: there sure is a better way but since this is an offline calculation we don't care for now
class IrradianceMapCalculation {

	vk::UniqueRenderPass m_renderPass;

	std::vector<vk::CommandBuffer> m_cmdBuffers;

	UniquePtr<RBuffer> m_cubeVertexBuffer;

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	std::array<vk::UniqueFramebuffer, 6> m_framebuffers;
	std::array<UniquePtr<RImageAttachment>, 6> m_faceAttachments;

	glm::mat4 m_captureProjection;
	std::array<glm::mat4, 6> m_captureViews;

	RDescriptorLayout m_skyboxDescLayout;
	vk::DescriptorSet m_descSet;

	GpuEnvironmentMap* m_envmapAsset;

	uint32 m_resolution;

	void MakeDesciptors();
	void MakeRenderPass();
	void AllocateCommandBuffers();
	void AllocateCubeVertexBuffer();
	void MakePipeline();
	void PrepareFaceInfo();
	void RecordAndSubmitCmdBuffers();
	void EditPods();

public:
	IrradianceMapCalculation(GpuEnvironmentMap* envmapAsset, uint32 calculationResolution);

	void Calculate();
};
} // namespace vl
