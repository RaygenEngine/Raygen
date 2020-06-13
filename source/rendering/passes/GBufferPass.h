#pragma once
#include "rendering/objects/GBuffer.h"
#include "rendering/scene/SceneGeometry.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class GBufferPass {

public:
	static vk::UniqueRenderPass CreateCompatibleRenderPass();

	static vk::UniquePipeline CreatePipeline(vk::PipelineLayout pipelineLayout, //
		std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages);

	static void RecordCmd(vk::CommandBuffer* cmdBuffer, GBuffer* gBuffer, //
		const std::vector<SceneGeometry*>& geometries);
};

} // namespace vl
