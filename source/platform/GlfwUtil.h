#pragma once

#include <vector>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>

namespace glfwutl {
std::vector<const char*> GetVulkanExtensions();

void SetupEventCallbacks(GLFWwindow* window);
} // namespace glfwutl
