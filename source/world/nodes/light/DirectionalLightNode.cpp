#include "pch/pch.h"

#include "world/nodes/light/DirectionalLightNode.h"

void DirectionalLightNode::RecalculateProjectionMatrix()
{
	m_projectionMatrix = glm::ortho(m_left, m_right, m_bottom, m_top, m_near, m_far);
	RecalculateViewProjectionMatrix();
}

void DirectionalLightNode::RecalculateViewMatrix()
{
	auto lookat = GetWorldTranslation() + GetFront();
	m_viewMatrix = glm::lookAt(GetWorldTranslation(), lookat, GetUp());
	RecalculateViewProjectionMatrix();
}

void DirectionalLightNode::RecalculateViewProjectionMatrix()
{
	m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
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
