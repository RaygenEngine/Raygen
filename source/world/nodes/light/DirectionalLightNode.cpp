#include "pch.h"

#include "world/nodes/light/DirectionalLightNode.h"
#include "asset/util/ParsingAux.h"


DirectionalLightNode::DirectionalLightNode(Node* parent)
	: LightNode(parent)
{
}

bool DirectionalLightNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
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


std::string DirectionalLightNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--DirectionalLight " + Node::ToString(verbose, depth);
}

