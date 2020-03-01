#pragma once


#include "renderer/Model.h"
#include <vulkan/vulkan.hpp>

// WIP:
class EditorPass {
public:
	void RecordCmd(vk::CommandBuffer* cmdBuffer);
};
