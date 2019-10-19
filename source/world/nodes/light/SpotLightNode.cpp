#include "pch/pch.h"

#include "world/nodes/light/SpotLightNode.h"

void SpotLightNode::RecalculateProjectionMatrix()
{
	auto ar = static_cast<float>(m_shadowMapWidth) / static_cast<float>(m_shadowMapHeight);
	m_projectionMatrix = glm::perspective(glm::radians(m_outerAperture), ar, m_near, m_far);
	RecalculateViewProjectionMatrix();
}

void SpotLightNode::RecalculateViewMatrix()
{
	auto lookat = GetWorldTranslation() + GetWorldForward();
	m_viewMatrix = glm::lookAt(GetWorldTranslation(), lookat, GetWorldUp());
	RecalculateViewProjectionMatrix();
}

void SpotLightNode::RecalculateViewProjectionMatrix()
{
	m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
}

void SpotLightNode::DirtyUpdate(DirtyFlagset flags)
{
	if (flags[DF::Aperture] || flags[DF::NearFar] || flags[DF::ShadowsTextSize]) {
		RecalculateProjectionMatrix();
	}

	if (flags[DF::TRS]) {
		RecalculateViewMatrix();
	}
}
