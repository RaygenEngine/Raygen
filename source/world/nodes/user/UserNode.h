#pragma once

#include "world/nodes/Node.h"

class UserNode : public Node {
	REFLECTED_NODE(UserNode, Node)
	{
		REFLECT_VAR(m_movementSpeed);
		REFLECT_VAR(m_turningSpeed);
	}

protected:
	// TODO: turning speed with delta is not working properly on different fps profiles
	float m_movementSpeed{ 10.0f };
	float m_turningSpeed{ 0.7f };

public:
	UserNode(Node* parent)
		: Node(parent)
	{
	}
};
