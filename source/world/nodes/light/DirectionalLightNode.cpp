#include "pch/pch.h"

#include "world/nodes/light/DirectionalLightNode.h"

void DirectionalLightNode::UpdateProjectionMatrix()
{
	m_projectionMatrix = glm::ortho(m_left, m_right, m_bottom, m_top, m_near, m_far);
}

void DirectionalLightNode::DirtyUpdate(DirtyFlagset flags)
{
	if (flags[DF::Projection] || flags[DF::NearFar]) {
		UpdateProjectionMatrix();
	}
}
