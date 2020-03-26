#include "pch.h"
#include "platform/Window.h"

#include "platform/GlfwUtl.h"

#include <glfw/glfw3.h>
#include <memory>

std::shared_ptr<Window::GlfwContext> glfwContext;


Window::Window(glm::vec2 size, const char* title)
{
	if (!glfwContext) {
		glfwContext = std::make_shared<Window::GlfwContext>();
	}
	m_glfwContext = glfwContext;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwCreateWindow(size.x, size.y, title, nullptr, nullptr);

	glfwutl::SetupEventCallbacks(m_window);
};

Window::~Window()
{
	glfwDestroyWindow(m_window);

	m_glfwContext.reset();
	if (glfwContext.use_count() == 1) {
		glfwContext.reset();
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

Window::GlfwContext::GlfwContext()
{
	glfwInit();
}

Window::GlfwContext::~GlfwContext()
{
	glfwTerminate();
}
