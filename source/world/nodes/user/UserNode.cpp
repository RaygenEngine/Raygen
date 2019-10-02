#include "pch.h"

#include "world/nodes/user/UserNode.h"

UserNode::UserNode(Node* parent)
	: Node(parent),
		m_movementSpeed(0.01f), 
		m_turningSpeed(0.003f)
{
}
