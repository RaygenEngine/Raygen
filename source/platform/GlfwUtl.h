#pragma once

struct GLFWwindow;

#include <vector>

namespace glfwutl {
std::vector<const char*> GetVulkanExtensions();

void SetupEventCallbacks(GLFWwindow* window);
} // namespace glfwutl
