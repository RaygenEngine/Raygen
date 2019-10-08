#include "pch.h"

#include "world/nodes/light/DirectionalLightNode.h"


DirectionalLightNode::DirectionalLightNode(Node* parent)
	: Node(parent)
	, m_projectionMatrix()
{
}

void DirectionalLightNode::DirtyUpdate()
{
	Node::DirtyUpdate();

	if (m_dirty[DF::Projection]) {
		UpdateProjectionMatrix();
	}
}
