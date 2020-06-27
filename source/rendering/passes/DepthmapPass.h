#pragma once
#include "rendering/wrappers/RDepthmap.h"
#include "rendering/scene/SceneGeometry.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class DepthmapPass {

public:
	static vk::UniqueRenderPass CreateCompatibleRenderPass();

	static size_t GetPushConstantSize();

	static vk::UniquePipeline CreatePipeline(vk::PipelineLayout pipelineLayout, //
		std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages);


	static void RecordCmd(vk::CommandBuffer* cmdBuffer, RDepthmap& depthmap, const glm::mat4& viewProj,
		const std::vector<SceneGeometry*>& geometries);
};

} // namespace vl
