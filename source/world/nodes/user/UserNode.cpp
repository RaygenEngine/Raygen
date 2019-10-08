#include "pch.h"

#include "world/nodes/user/UserNode.h"

UserNode::UserNode(Node* parent)
	: Node(parent)
	, m_movementSpeed(10.0f)
	, m_turningSpeed(0.7f)
{
}
