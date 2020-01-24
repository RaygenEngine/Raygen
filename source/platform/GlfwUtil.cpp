#include "GlfwUtil.h"
#include <glfw/glfw3.h>

std::vector<const char*> glfwutl::GetVulkanExtensions()
{
	std::vector<const char*> extensions;
	uint32 size;
	const char** c_ext = glfwGetRequiredInstanceExtensions(&size);

	for (uint32 i = 0; i < size; ++i) {
		extensions.push_back(c_ext[i]);
	}

	return extensions;
}
