#include "pch.h"

#include "world/nodes/light/SpotLightNode.h"


SpotLightNode::SpotLightNode(Node* parent)
	: Node(parent)
	, m_projectionMatrix()
	, m_aperture(45.f)
{
}

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
