#pragma once
#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/DescriptorSetLayout.h"
#include "rendering/wrappers/ImageView.h"

namespace vl {

// CHECK: there sure is a better way but since this is an offline calculation we don't care for now
class IrradianceMapCalculation {

	vk::UniqueRenderPass m_renderPass;

	RBuffer m_cubeVertexBuffer;

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	std::array<vk::UniqueFramebuffer, 6> m_framebuffer;
	std::array<RImageAttachment, 6> m_faceAttachments;

	glm::mat4 m_captureProjection;
	std::array<glm::mat4, 6> m_captureViews;

	RDescriptorSetLayout m_skyboxDescLayout;
	vk::DescriptorSet m_descSet;

	GpuEnvironmentMap* m_envmapAsset;

	uint32 m_resolution;

	void MakeDesciptors();
	void MakeRenderPass();
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
