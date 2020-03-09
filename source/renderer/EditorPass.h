#pragma once
#include <vulkan/vulkan.hpp>

class EditorPass {
public:
	void RecordCmd(vk::CommandBuffer* cmdBuffer);
};
