#pragma once
#include "rendering/wrappers/Buffer.h"
#include "rendering/wrappers/DescriptorSetLayout.h"
#include "rendering/wrappers/ImageView.h"

struct SceneReflprobe;

namespace vl {
struct CubemapMipFrames {
	std::array<vk::UniqueFramebuffer, 6> framebuffers;
	std::vector<vk::UniqueImageView> faceViews;
};

class PrefilteredMapCalculation {
public:
	PrefilteredMapCalculation(SceneReflprobe* rp);

	void RecordPass(vk::CommandBuffer cmdBuffer, vk::DescriptorSet surroundingCubeDescSet, uint32 resolution);
	void Resize(const RCubemap& sourceCubemap, uint32 resolution);

private:
	SceneReflprobe* m_reflprobe{ nullptr };

	vk::UniqueRenderPass m_renderPass;
	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	RBuffer m_cubeVertexBuffer;

	std::array<CubemapMipFrames, 6> m_cubemapMips;

	void MakeRenderPass();
	void AllocateCubeVertexBuffer();
	void MakePipeline();
};
} // namespace vl
