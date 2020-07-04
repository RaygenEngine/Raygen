#pragma once
#include "rendering/wrappers/RGbuffer.h"
#include "rendering/scene/SceneGeometry.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class AnimatedGbufferPass {

public:
	static vk::UniqueRenderPass CreateCompatibleRenderPass();

	static size_t GetPushConstantSize();

	static vk::UniquePipeline CreatePipeline(vk::PipelineLayout pipelineLayout, //
		std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages);

	static void RecordCmd(vk::CommandBuffer* cmdBuffer, RGbuffer* gbuffer, //
		const std::vector<SceneAnimatedGeometry*>& geometries);
};

} // namespace vl
