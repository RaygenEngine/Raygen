#include "pch.h"

#include "world/nodes/light/DirectionalLightNode.h"

void DirectionalLightNode::DirtyUpdate()
{
	Node::DirtyUpdate();

	if (m_dirty[DF::Projection]) {
		UpdateProjectionMatrix();
	}
}
