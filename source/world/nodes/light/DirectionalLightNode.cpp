#include "pch/pch.h"

#include "world/nodes/light/DirectionalLightNode.h"
#include "core/MathAux.h"

void DirectionalLightNode::CalculateWorldAABB()
{
	m_aabb = m_frustum.FrustumPyramidAABB(GetWorldTranslation());
}

void DirectionalLightNode::RecalculateProjectionMatrix()
{
	m_projectionMatrix = glm::ortho(m_left, m_right, m_bottom, m_top, m_near, m_far);

	RecalculateViewProjectionMatrix();
}

void DirectionalLightNode::RecalculateViewMatrix()
{
	auto lookat = GetWorldTranslation() + GetWorldForward();
	m_viewMatrix = glm::lookAt(GetWorldTranslation(), lookat, GetWorldUp());

	RecalculateViewProjectionMatrix();
}

void DirectionalLightNode::RecalculateViewProjectionMatrix()
{
	m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;

	RecalculateFrustum();
}

void DirectionalLightNode::RecalculateFrustum()
{
	// viewProj to get frustum plane equations in world space
	m_frustum.ExtractFromMatrix(m_viewProjectionMatrix);
	CalculateWorldAABB();
}

void DirectionalLightNode::DirtyUpdate(DirtyFlagset flags)
{
	if (flags[DF::OrthoSides] || flags[DF::NearFar]) {
		RecalculateProjectionMatrix();
	}

	if (flags[DF::TRS]) {
		RecalculateViewMatrix();
	}
}
