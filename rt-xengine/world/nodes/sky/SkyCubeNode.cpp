#include "pch.h"

#include "world/nodes/sky/SkyCubeNode.h"
#include "assets/other/xml/ParsingAux.h"
#include "assets/AssetManager.h"


SkyCubeNode::SkyCubeNode(Node* parent)
	: Node(parent)
{
}

bool SkyCubeNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
{
	Node::LoadAttributesFromXML(xmlData);

	if (AttributeExists(xmlData, "cube_map"))
	{
		m_cubeMap = GetDiskAssetManager()->LoadCubeMapAsset(xmlData->Attribute("cube_map"), DynamicRange::LOW, false);

		if (!m_cubeMap)
			return false;
	}

	return true;
}

std::string SkyCubeNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--SkyCube " + Node::ToString(verbose, depth);
}
