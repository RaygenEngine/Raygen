#include "pch/pch.h"

#include "world/nodes/camera/WindowCameraNode.h"
#include "system/Engine.h"
#include <glfw/glfw3.h>

WindowCameraNode::WindowCameraNode()
	: CameraNode()
{
	m_resizeListener.BindMember(this, &WindowCameraNode::WindowResize);
	auto mainWindow = Engine::GetMainWindow();

	int32 width;
	int32 height;
	glfwGetWindowSize(mainWindow, &width, &height);

	if (mainWindow && width > 0 && height > 0) {
		m_viewportWidth = width;
		m_viewportHeight = height;
	}
}

void WindowCameraNode::WindowResize(int32 width, int32 height)
{
	m_viewportWidth = width;
	m_viewportHeight = height;
	SetDirty(DF::ViewportSize);
}
