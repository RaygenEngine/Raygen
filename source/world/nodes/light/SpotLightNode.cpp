#include "pch.h"

#include "world/nodes/light/SpotLightNode.h"
#include "asset/util/ParsingAux.h"


SpotLightNode::SpotLightNode(Node* parent)
	: LightNode(parent)
{
}

std::string SpotLightNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--SpotLight " + Node::ToString(verbose, depth);
}


bool SpotLightNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
{
	LightNode::LoadAttributesFromXML(xmlData);

	glm::vec3 localLookat{};
	if (ParsingAux::ReadFloatsAttribute(xmlData, "lookat", localLookat))
	{
		// if lookat read overwrite following
		SetLocalOrientation(utl::GetOrientationFromLookAtAndPosition(localLookat, GetLocalTranslation()));
	}

	return true;
}


