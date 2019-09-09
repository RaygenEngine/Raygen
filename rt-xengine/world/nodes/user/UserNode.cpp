#include "pch.h"

#include "world/nodes/user/UserNode.h"
#include "assets/other/xml/ParsingAux.h"

UserNode::UserNode(Node* parent)
	: Node(parent),
		m_movementSpeed(0.01f), 
		m_turningSpeed(0.003f)
{
	REFLECT_VAR(m_movementSpeed);
	REFLECT_VAR(m_turningSpeed);
}
