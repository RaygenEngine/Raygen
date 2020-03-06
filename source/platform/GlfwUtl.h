#pragma once

#include <glfw/glfw3.h>
#include <vector>

namespace glfwutl {
std::vector<const char*> GetVulkanExtensions();

void SetupEventCallbacks(GLFWwindow* window);
} // namespace glfwutl
