#pragma once


#include "renderer/Model.h"
#include <vulkan/vulkan.hpp>

class EditorPass {
public:
	void RecordCmd(vk::CommandBuffer* cmdBuffer);
};
