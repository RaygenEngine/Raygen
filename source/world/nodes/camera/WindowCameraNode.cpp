#include "pch/pch.h"

#include "world/nodes/camera/WindowCameraNode.h"
#include "platform/windows/Win32Window.h"

WindowCameraNode::WindowCameraNode(Node* parent)
	: CameraNode(parent)
{
	m_resizeListener.BindMember(this, &WindowCameraNode::WindowResize);
}

void WindowCameraNode::WindowResize(int32 width, int32 height)
{
	m_viewportWidth = width;
	m_viewportHeight = height;
	SetDirty(DF::ViewportSize);
}
