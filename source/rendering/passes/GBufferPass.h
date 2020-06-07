#pragma once
#include "rendering/assets/GpuMesh.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/scene/SceneGeometry.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class GBufferPass {
	friend class GBuffer;


public:
	vk::UniqueRenderPass m_renderPass;
	vk::UniquePipelineLayout m_pipelineLayout;

public:
	GBufferPass();

	void RecordCmd(vk::CommandBuffer* cmdBuffer, GBuffer* gBuffer, const std::vector<SceneGeometry*>& geometries);

	UniquePtr<GBuffer> CreateCompatibleGBuffer(uint32 width, uint32 height);
};

} // namespace vl
