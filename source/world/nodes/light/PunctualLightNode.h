#pragma once

#include "world/nodes/Node.h"
// TODO:
class PunctualLightNode : public Node {
	REFLECTED_NODE(PunctualLightNode, Node) {}

	glm::vec3 m_color;
	float m_intensity;

	bool m_hasShadow;

public:
	PunctualLightNode::PunctualLightNode(Node* parent)
		: Node(parent)
	{
	}
};
