#include "pch.h"

#include "world/nodes/light/SpotLightNode.h"
#include "asset/util/ParsingAux.h"


SpotLightNode::SpotLightNode(Node* parent)
	: LightNode(parent)
{
}


bool SpotLightNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
{
	LightNode::LoadAttributesFromXML(xmlData);
	// WIP:


	return true;
}


