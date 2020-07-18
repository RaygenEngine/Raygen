#pragma once
#include "rendering/wrappers/RDescriptorLayout.h"
#include "rendering/wrappers/RImageAttachment.h"

namespace vl {
class RBuffer;

struct CubemapMipFrames {
	std::array<vk::UniqueFramebuffer, 6> framebuffers;
	std::array<UniquePtr<RImageAttachment>, 6> faceAttachments;
};

class PrefilteredMapCalculation {

	vk::UniqueRenderPass m_renderPass;

	std::vector<vk::CommandBuffer> m_cmdBuffers;

	UniquePtr<RBuffer> m_cubeVertexBuffer;

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	std::array<CubemapMipFrames, 6> m_cubemapMips;

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
	PrefilteredMapCalculation(GpuEnvironmentMap* envmapAsset, uint32 calculationResolution);

	void Calculate();
};
} // namespace vl
