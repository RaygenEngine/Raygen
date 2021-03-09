#pragma once

struct GLFWwindow;

#include <vector>

namespace glfwutl {
void SetupEventCallbacks(GLFWwindow* window);

void SetMousePos(int32 x, int32 y);
} // namespace glfwutl
