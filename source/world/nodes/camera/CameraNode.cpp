#include "pch/pch.h"

#include "world/nodes/camera/CameraNode.h"
#include "platform/windows/Win32Window.h"

void CameraNode::RecalculateProjectionFov()
{
	auto ar = static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight);

	m_projectionMatrix = glm::perspective(glm::radians(m_vFov), ar, m_near, m_far);
	m_hFov = glm::degrees(2 * atan(ar * tan(glm::radians(m_vFov) * 0.5f)));


	SetDirty(DF::Projection);
}

void CameraNode::DirtyUpdate(DirtyFlagset flags)
{
	if (flags[DF::Projection] || flags[DF::ViewportSize]) {
		RecalculateProjectionFov();
	}
}
