#include "pch/pch.h"

#include "world/nodes/light/SpotLightNode.h"
#include "core/MathAux.h"

void SpotLightNode::CalculateWorldAABB()
{
	m_aabb = m_frustum.FrustumPyramidAABB(GetTranslation());
}

void SpotLightNode::RecalculateProjectionMatrix()
{
	const auto ar = static_cast<float>(m_shadowMapWidth) / static_cast<float>(m_shadowMapHeight);
	m_projectionMatrix = glm::perspective(m_outerAperture, ar, m_near, m_far);

	RecalculateViewProjectionMatrix();
}

void SpotLightNode::RecalculateViewMatrix()
{
	const auto lookAt = GetTranslation() + GetForward();
	m_viewMatrix = glm::lookAt(GetTranslation(), lookAt, GetUp());

	RecalculateViewProjectionMatrix();
}

void SpotLightNode::RecalculateViewProjectionMatrix()
{
	m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;

	RecalculateFrustum();
}

void SpotLightNode::RecalculateFrustum()
{
	// viewProj to get frustum plane equations in world space
	m_frustum.ExtractFromMatrix(m_viewProjectionMatrix);
	CalculateWorldAABB();
}

void SpotLightNode::DirtyUpdate(DirtyFlagset flags)
{
	if (flags[DF::Aperture] || flags[DF::NearFar] || flags[DF::ShadowsTextSize]) {
		RecalculateProjectionMatrix();
	}

	if (flags[DF::SRT]) {
		RecalculateViewMatrix();
	}
}
