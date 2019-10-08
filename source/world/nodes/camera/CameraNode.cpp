#include "pch.h"

#include "world/nodes/camera/CameraNode.h"
#include "platform/windows/Win32Window.h"

CameraNode::CameraNode(Node* parent)
	: Node(parent),
		m_focalLength(1.f),
		m_vFov(60.f),
		m_hFov(45.f),
		m_near(0.2f),
		m_far(1000.0f),
		m_projectionMatrix()
{}

void CameraNode::RecalculateProjectionFov()
{
	auto ar = static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight);

	m_projectionMatrix = glm::perspective(glm::radians(m_vFov), ar, m_near, m_far);
	m_hFov = glm::degrees(2 * atan(ar * tan(glm::radians(m_vFov) * 0.5f)));
	m_dirty.set(DF::Projection);
}

void CameraNode::DirtyUpdate()
{
	Node::DirtyUpdate();
	if (m_dirty[DF::Projection] || m_dirty[DF::ViewportSize])
	{
		RecalculateProjectionFov();
	}
}