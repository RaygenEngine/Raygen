#pragma once
#include "rendering/assets/GpuMesh.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/objects/GBuffer.h"
#include "rendering/objects/RDepthmap.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/scene/SceneGeometry.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class ShadowmapPass {
	vk::UniqueRenderPass m_renderPass;

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

public:
	ShadowmapPass();
	void MakePipeline();

	void RecordCmd(vk::CommandBuffer* cmdBuffer, RDepthmap* depthmap, const glm::mat4& viewProj,
		const std::vector<SceneGeometry*>& geometries);

	[[nodiscard]] vk::RenderPass GetRenderPass() const { return m_renderPass.get(); }
};

} // namespace vl
