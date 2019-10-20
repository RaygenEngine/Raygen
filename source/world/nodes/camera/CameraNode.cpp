#include "pch/pch.h"

#include "world/nodes/camera/CameraNode.h"
#include "core/MathAux.h"

void CameraNode::RecalculateProjectionFov()
{
	auto ar = static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight);

	m_projectionMatrix = glm::perspective(glm::radians(m_vFov), ar, m_near, m_far);
	m_hFov = glm::degrees(2 * atan(ar * tan(glm::radians(m_vFov) * 0.5f)));
	RecalculateViewProjectionMatrix();
}

void CameraNode::RecalculateViewMatrix()
{
	m_viewMatrix = glm::lookAt(GetWorldTranslation(), GetLookAt(), GetWorldUp());
	RecalculateViewProjectionMatrix();
}

void CameraNode::RecalculateViewProjectionMatrix()
{
	m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
	RecalculateFrustum();
}

void CameraNode::RecalculateFrustum()
{
	// viewProj to get frustum plane equations in world space
	math::ExtractFrustumPlanes(m_frustum, m_viewProjectionMatrix);
}

void CameraNode::DirtyUpdate(DirtyFlagset flags)
{
	if (flags[DF::Projection] || flags[DF::ViewportSize]) {
		RecalculateProjectionFov();
	}

	if (flags[DF::TRS] || flags[DF::FocalLength]) {
		RecalculateViewMatrix();
	}
}
