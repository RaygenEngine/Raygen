#include "pch.h"
#include "WindowCameraNode.h"

#include "engine/Engine.h"
#include "platform/Platform.h"

#include <glfw/glfw3.h>

WindowCameraNode::WindowCameraNode()
	: CameraNode()
{
	Event::OnWindowResize.Bind(this, &WindowCameraNode::OnWindowResizeEvent);
	Event::OnViewportUpdated.Bind(this, &WindowCameraNode::OnViewportUpdatedEvent);


	auto size = Platform::GetMainSize();
	int32 width = size.x;
	int32 height = size.y;

	if (width > 0 && height > 0) {
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

void WindowCameraNode::OnWindowResizeEvent(int32 width, int32 height)
{
	WindowResize(width, height);
}

void WindowCameraNode::OnViewportUpdatedEvent()
{
	WindowResize(math::roundToInt(g_ViewportCoordinates.size.x), math::roundToInt(g_ViewportCoordinates.size.y));
}
