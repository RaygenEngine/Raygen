#pragma once
#include <vulkan/vulkan.hpp>

namespace vl {
class EditorPass {
public:
	void RecordCmd(vk::CommandBuffer* cmdBuffer);
};
} // namespace vl
