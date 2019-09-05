#include "pch.h"

#include "world/nodes/user/UserNode.h"
#include "assets/other/xml/ParsingAux.h"

UserNode::UserNode(Node* parent)
	: Node(parent),
		m_movementSpeed(0.01f), 
		m_turningSpeed(0.003f)
{
}

bool UserNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
{
	Node::LoadAttributesFromXML(xmlData);

	Assets::ReadFloatsAttribute<float>(xmlData, "movement_speed", m_movementSpeed);
	Assets::ReadFloatsAttribute<float>(xmlData, "turning_speed", m_turningSpeed);

	return true;
}
