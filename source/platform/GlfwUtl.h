#pragma once

struct GLFWwindow;

#include <vector>

namespace glfwutl {
std::vector<const char*> GetVulkanExtensions();

void SetupEventCallbacks(GLFWwindow* window);

void SetMousePos(int32 x, int32 y);
} // namespace glfwutl
