#pragma once

#include "world/nodes/Node.h"
// TODO:
class PunctualLightNode : public Node {
	REFLECTED_NODE(PunctualLightNode, Node) {}

	glm::vec3 m_color{ glm::vec3(1.f) };
	float m_intensity{ 10.f };

	bool m_hasShadow{ true };

public:
	PunctualLightNode(Node* parent)
		: Node(parent)
	{
	}
};
