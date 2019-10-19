#include "pch/pch.h"

#include "world/nodes/light/PunctualLightNode.h"

void PunctualLightNode::RecalculateProjectionMatrix()
{
	auto ar = static_cast<float>(m_shadowMapWidth) / static_cast<float>(m_shadowMapHeight);
	m_projectionMatrix = glm::perspective(glm::radians(90.0f), ar, m_near, m_far);
	RecalculateViewProjectionMatrices();
}

void PunctualLightNode::RecalculateViewMatrices()
{
	auto lookat = GetWorldTranslation() + glm::vec3(1.0, 0.0, 0.0);
	m_viewMatrices[0] = glm::lookAt(GetWorldTranslation(), lookat, glm::vec3(0.0, -1.0, 0.0));
	lookat = GetWorldTranslation() + glm::vec3(-1.0, 0.0, 0.0);
	m_viewMatrices[1] = glm::lookAt(GetWorldTranslation(), lookat, glm::vec3(0.0, -1.0, 0.0));
	lookat = GetWorldTranslation() + glm::vec3(0.0, 1.0, 0.0);
	m_viewMatrices[2] = glm::lookAt(GetWorldTranslation(), lookat, glm::vec3(0.0, 0.0, 1.0));
	lookat = GetWorldTranslation() + glm::vec3(0.0, -1.0, 0.0);
	m_viewMatrices[3] = glm::lookAt(GetWorldTranslation(), lookat, glm::vec3(0.0, 0.0, -1.0));
	lookat = GetWorldTranslation() + glm::vec3(0.0, 0.0, 1.0);
	m_viewMatrices[4] = glm::lookAt(GetWorldTranslation(), lookat, glm::vec3(0.0, -1.0, 0.0));
	lookat = GetWorldTranslation() + glm::vec3(0.0, 0.0, -1.0);
	m_viewMatrices[5] = glm::lookAt(GetWorldTranslation(), lookat, glm::vec3(0.0, -1.0, 0.0));
	RecalculateViewProjectionMatrices();
}

void PunctualLightNode::RecalculateViewProjectionMatrices()
{
	m_viewProjectionMatrices[0] = m_projectionMatrix * m_viewMatrices[0];
	m_viewProjectionMatrices[1] = m_projectionMatrix * m_viewMatrices[1];
	m_viewProjectionMatrices[2] = m_projectionMatrix * m_viewMatrices[2];
	m_viewProjectionMatrices[3] = m_projectionMatrix * m_viewMatrices[3];
	m_viewProjectionMatrices[4] = m_projectionMatrix * m_viewMatrices[4];
	m_viewProjectionMatrices[5] = m_projectionMatrix * m_viewMatrices[5];
}

void PunctualLightNode::DirtyUpdate(DirtyFlagset flags)
{
	if (flags[DF::NearFar] || flags[DF::ShadowsTextSize]) {
		RecalculateProjectionMatrix();
	}

	if (flags[DF::TRS]) {
		RecalculateViewMatrices();
	}
}
