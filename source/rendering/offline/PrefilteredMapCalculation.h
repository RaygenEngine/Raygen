#pragma once
#include "assets/pods/EnvironmentMap.h"
#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/objects/RBuffer.h"
#include "rendering/objects/ImageAttachment.h"

#include <vulkan/vulkan.hpp>
namespace vl {

struct CubemapMipFrames {
	std::array<vk::UniqueFramebuffer, 6> framebuffers;
	std::array<UniquePtr<ImageAttachment>, 6> faceAttachments;
};

class PrefilteredMapCalculation {

	vk::UniqueRenderPass m_renderPass;

	std::vector<vk::CommandBuffer> m_cmdBuffers;

	UniquePtr<vl::RBuffer> m_cubeVertexBuffer;

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	std::array<CubemapMipFrames, 6> m_cubemapMips;

	glm::mat4 m_captureProjection;
	std::array<glm::mat4, 6> m_captureViews;

	RDescriptorLayout m_skyboxDescLayout;
	vk::DescriptorSet m_descSet;

	EnvironmentMap::Gpu* m_envmapAsset;

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
	PrefilteredMapCalculation(EnvironmentMap::Gpu* envmapAsset, uint32 calculationResolution);

	void Calculate();
};
} // namespace vl
