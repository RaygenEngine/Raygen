#include "pch.h"
#include "Platform.h"

#include "platform/GlfwUtl.h"

std::vector<const char*> Platform::GetVulkanExtensions()
{
	return glfwutl::GetVulkanExtensions();
}
