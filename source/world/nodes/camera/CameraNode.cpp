#include "pch/pch.h"

#include "world/nodes/camera/CameraNode.h"
#include "core/MathAux.h"

void CameraNode::RecalculateProjectionFov()
{
	auto ar = static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight);
	m_hFov = glm::degrees(2 * atan(ar * tan(glm::radians(m_vFov) * 0.5f)));
	// m_projectionMatrix = glm::perspective(glm::radians(m_vFov), ar, m_near, m_far);
	// m_hFov = glm::degrees(2 * atan(ar * tan(glm::radians(m_vFov) * 0.5f)));

	float top = tan(glm::radians(m_vFov / 2.f + m_vFovOffset)) * m_near;
	float bottom = tan(-glm::radians(m_vFov / 2.f - m_vFovOffset)) * m_near;

	float right = tan(glm::radians(m_hFov / 2.f + m_hFovOffset)) * m_near;
	float left = tan(-glm::radians(m_hFov / 2.f - m_hFovOffset)) * m_near;

	m_projectionMatrix = glm::frustum(left, right, bottom, top, m_near, m_far);


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
	m_frustum.ExtractFromMatrix(m_viewProjectionMatrix);
}

void CameraNode::DirtyUpdate(DirtyFlagset flags)
{
	Node::DirtyUpdate(flags);
	if (flags[DF::Projection] || flags[DF::ViewportSize]) {
		RecalculateProjectionFov();
	}

	if (flags[DF::TRS] || flags[DF::FocalLength]) {
		RecalculateViewMatrix();
	}
}
