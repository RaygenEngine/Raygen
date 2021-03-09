#include "Window.h"

#include "platform/GlfwUtl.h"

#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>

inline size_t g_currentWindows{ 0 };

Window::Window(WindowCreationParams params)
{
	if (++g_currentWindows == 1) {
		glfwInit();
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_AUTOBORDERLESS, GLFW_TRUE);

	m_window = glfwCreateWindow(params.size.x, params.size.y, params.title, nullptr, nullptr);

	// Restore state for ImGui windows. (ImGui glfw cannot explicitly store & reset the state before making their
	// windows because it does not know about autoborderless extension of glfw)
	// We might want to impelment auto borderless mode in imgui popups for aero snapping eventually, through
	// imgui_impl_glfw
	glfwWindowHint(GLFW_AUTOBORDERLESS, GLFW_FALSE);

	glfwPollEvents(); // drop current events in the queue
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

HWND Window::GetNativeHandle() const
{
	return glfwGetWin32Window(m_window);
}
