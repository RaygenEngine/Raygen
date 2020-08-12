#pragma once
#include <vulkan/vulkan.hpp>

struct SceneRenderDesc;

namespace vl {
class CopyHdrTexture {

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

public:
	void MakePipeline(vk::RenderPass outRp);
	void RecordCmd(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc);
};
} // namespace vl
