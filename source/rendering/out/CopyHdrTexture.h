#pragma once
#include <vulkan/vulkan.hpp>

namespace vl {
class CopyHdrTexture {

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

public:
	void MakePipeline();
	void RecordCmd(vk::CommandBuffer* cmdBuffer);
};
} // namespace vl
