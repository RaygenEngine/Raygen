#include "pch/pch.h"

#include "world/nodes/light/SpotLightNode.h"

void SpotLightNode::UpdateProjectionMatrix()
{
	auto ar = static_cast<float>(m_shadowMapWidth) / static_cast<float>(m_shadowMapHeight);
	m_projectionMatrix = glm::perspective(glm::radians(m_aperture), ar, m_near, m_far);
}

void SpotLightNode::DirtyUpdate()
{
	Node::DirtyUpdate();

	if (m_dirty[DF::Projection]) {
		UpdateProjectionMatrix();
	}
}
