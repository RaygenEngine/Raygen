#pragma once
#include "rendering/objects/Buffer.h"
#include "rendering/objects/ImageAttachment.h"
#include "rendering/assets/GpuCubemap.h"

#include <vulkan/vulkan.hpp>
namespace vl {
// CHECK: there sure is a better way but since this is an offline calculation we don't care for now
class IrradianceMapCalculation {

	vk::UniqueRenderPass m_renderPass;

	std::vector<vk::CommandBuffer> m_cmdBuffers;

	UniquePtr<vl::RawBuffer> m_cubeVertexBuffer;

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	std::array<vk::UniqueFramebuffer, 6> m_framebuffers;
	std::array<UniquePtr<ImageAttachment>, 6> m_faceAttachments;

	glm::mat4 m_captureProjection;
	std::array<glm::mat4, 6> m_captureViews;

	DescriptorLayout m_skyboxDescLayout;
	vk::DescriptorSet m_descSet;

	::Cubemap::Gpu* m_cubemapAsset;

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
	IrradianceMapCalculation(::Cubemap::Gpu* cubemapAsset, uint32 calculationResolution = 512);

	void Calculate();
};
} // namespace vl
