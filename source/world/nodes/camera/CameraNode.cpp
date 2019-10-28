#include "pch/pch.h"

#include "world/nodes/camera/CameraNode.h"
#include "core/MathAux.h"

void CameraNode::CalculateWorldAABB()
{
	m_aabb = m_frustum.FrustumPyramidAABB(GetTranslation());
}

void CameraNode::RecalculateProjectionFov()
{
	const auto ar = static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight);
	m_hFov = 2 * atan(ar * tan(m_vFov * 0.5f));

	const auto top = tan(m_vFov / 2.f + m_vFovOffset) * m_near;
	const auto bottom = tan(-m_vFov / 2.f - m_vFovOffset) * m_near;

	const auto right = tan(m_hFov / 2.f + m_hFovOffset) * m_near;
	const auto left = tan(-m_hFov / 2.f - m_hFovOffset) * m_near;

	m_projectionMatrix = glm::frustum(left, right, bottom, top, m_near, m_far);


	RecalculateViewProjectionMatrix();
}

void CameraNode::RecalculateViewMatrix()
{
	m_viewMatrix = glm::lookAt(GetTranslation(), GetLookAt(), GetUp());
	RecalculateViewProjectionMatrix();
}

void CameraNode::RecalculateViewProjectionMatrix()
{
	m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;

	// viewProj to get frustum plane equations in world space
	m_frustum.ExtractFromMatrix(m_viewProjectionMatrix);
	CalculateWorldAABB();
}

void CameraNode::DirtyUpdate(DirtyFlagset flags)
{
	Node::DirtyUpdate(flags);
	if (flags[DF::Projection] || flags[DF::ViewportSize]) {
		RecalculateProjectionFov();
	}

	if (flags[DF::SRT] || flags[DF::FocalLength]) {
		RecalculateViewMatrix();
	}
}
