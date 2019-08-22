#include "pch.h"
#include "LightNode.h"
#include "assets/other/xml/ParsingAux.h"

#include "world/World.h"

namespace World
{
	LightNode::LightNode(Node* parent)
		: Node(parent),
		  m_color()
	{
	}

	std::string LightNode::ToString(bool verbose, uint depth) const
	{
		return std::string("    ") * depth + "|--light " + Node::ToString(verbose, depth);
	}

	//void LightNode::OnUpdate(const CTickEvent& event)
	//{
	//	//auto& keyboard = GetKeyboard();

	//	//if (keyboard.IsKeyRepeat(XVK_UP))
	//	//	MoveFront(0.005f * GetWorld()->GetDeltaTime());

	//	//if (keyboard.IsKeyRepeat(XVK_DOWN))
	//	//	MoveBack(0.005f * GetWorld()->GetDeltaTime());

	//	//if (keyboard.IsKeyRepeat(XVK_RIGHT))
	//	//	MoveRight(0.005f * GetWorld()->GetDeltaTime());

	//	//if (keyboard.IsKeyRepeat(XVK_LEFT))
	//	//	MoveLeft(0.005f * GetWorld()->GetDeltaTime());

	//	////if (keyboard.IsKeyRepeat(XVK_KP_8))
	//	////	MoveUp(0.005f * GetWorld()->GetDeltaTime());

	//	////if (keyboard.IsKeyRepeat(XVK_KP_2))
	//	////	MoveDown(0.005f * GetWorld()->GetDeltaTime());
	//}

	bool LightNode::LoadAttributesFromXML(const tinyxml2::XMLElement * xmlData)
	{
		Node::LoadAttributesFromXML(xmlData);

		Assets::ReadFloatsAttribute(xmlData, "color", m_color);

		return true;
	}
}
