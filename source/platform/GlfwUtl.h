#pragma once

#include "engine/Engine.h"

#include <vector>

namespace glfwutl {
std::vector<const char*> GetVulkanExtensions();

void SetupEventCallbacks(GLFWwindow* window);
} // namespace glfwutl
