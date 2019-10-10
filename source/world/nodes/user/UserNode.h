#pragma once

#include "world/nodes/Node.h"

class UserNode : public Node {
	REFLECTED_NODE(UserNode, Node)
	{
		REFLECT_VAR(m_movementSpeed);
		REFLECT_VAR(m_turningSpeed);
	}

protected:
	float m_movementSpeed{ 10.0f };
	float m_turningSpeed{ 0.7f };

public:
	UserNode(Node* parent)
		: Node(parent)
	{
	}
};
