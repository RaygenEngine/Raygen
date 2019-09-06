#include "pch.h"

#include "world/nodes/light/LightNode.h"
#include "assets/other/xml/ParsingAux.h"


LightNode::LightNode(Node* parent)
	: Node(parent),
		m_color()
{
}

std::string LightNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--light " + Node::ToString(verbose, depth);
}

bool LightNode::LoadAttributesFromXML(const tinyxml2::XMLElement * xmlData)
{
	Node::LoadAttributesFromXML(xmlData);

	ParsingAux::ReadFloatsAttribute(xmlData, "color", m_color);

	return true;
}
