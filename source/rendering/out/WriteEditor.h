#pragma once
#include <vulkan/vulkan.hpp>

namespace vl {
class WriteEditor {
public:
	void RecordCmd(vk::CommandBuffer* cmdBuffer);
};
} // namespace vl
