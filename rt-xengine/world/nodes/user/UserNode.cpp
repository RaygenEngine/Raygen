#include "pch.h"

#include "UserNode.h"
#include "world/World.h"

namespace World
{
	UserNode::UserNode(Node* parent)
		: Node(parent),
		  m_movementSpeed(0.01f), 
		  m_turningSpeed(0.003f)
	{
		REFLECT_VAR(m_movementSpeed);
		REFLECT_VAR(m_turningSpeed);
	}

	bool UserNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
	{
		Node::LoadAttributesFromXML(xmlData);

		Assets::ReadFloatsAttribute<float>(xmlData, "movement_speed", m_movementSpeed);
		Assets::ReadFloatsAttribute<float>(xmlData, "turning_speed", m_turningSpeed);

		return true;
	}
}
