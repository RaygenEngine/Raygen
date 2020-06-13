#pragma once

#include <vulkan/vulkan.hpp>

namespace vl {
// TODO:
class PostprocessPass {
	vk::UniqueRenderPass m_renderPass;

public:
	PostprocessPass();
	void RecordCmd();
};

} // namespace vl
