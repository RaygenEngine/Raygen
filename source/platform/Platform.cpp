#include "Platform.h"

#include "platform/GlfwUtl.h"

#include <glfw/glfw3.h>

Window* MainWindow{};

void Platform::Init(WindowCreationParams mainWindowParams)
{
	MainWindow = new Window(mainWindowParams);
}

void Platform::Destroy()
{
	delete MainWindow;
}

std::vector<const char*> Platform::GetVulkanExtensions()
{
	return glfwutl::GetVulkanExtensions();
}

Window* Platform::GetMainWindow()
{
	return MainWindow;
}

void Platform::PollEvents()
{
	glfwPollEvents();
}
