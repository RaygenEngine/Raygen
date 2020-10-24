#pragma once
#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/DescriptorSetLayout.h"
#include "rendering/wrappers/ImageView.h"

namespace vl {

struct CubemapMipFrames {
	std::array<vk::UniqueFramebuffer, 6> framebuffers;
	std::array<RImageAttachment, 6> faceAttachments;
};

class PrefilteredMapCalculation {

	vk::UniqueRenderPass m_renderPass;

	RBuffer m_cubeVertexBuffer;

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	std::array<CubemapMipFrames, 6> m_cubemapMips;

	glm::mat4 m_captureProjection;
	std::array<glm::mat4, 6> m_captureViews;

	GpuEnvironmentMap* m_envmapAsset;

	uint32 m_resolution;

	void MakeRenderPass();
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
