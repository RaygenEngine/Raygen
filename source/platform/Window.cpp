#include "pch.h"
#include "Window.h"

#include "platform/GlfwUtl.h"

#include <glfw/glfw3.h>
#include <memory>

inline size_t g_currentWindows{ 0 };

Window::Window(WindowCreationParams params)
{
	if (++g_currentWindows == 1) {
		glfwInit();
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window = glfwCreateWindow(params.size.x, params.size.y, params.title, nullptr, nullptr);

	glfwutl::SetupEventCallbacks(m_window);
};

Window::~Window()
{
	glfwDestroyWindow(m_window);

	if (--g_currentWindows == 0) {
		glfwTerminate();
	}
}

glm::uvec2 Window::GetSize() const
{
	int32 width{};
	int32 height{};
	glfwGetWindowSize(m_window, &width, &height);
	return glm::uvec2(width, height);
}

void Window::SetTitle(const char* title)
{
	glfwSetWindowTitle(m_window, title);
}
