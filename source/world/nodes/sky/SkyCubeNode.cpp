#include "pch.h"

#include "world/nodes/sky/SkyCubeNode.h"
#include "asset/util/ParsingAux.h"
#include "asset/AssetManager.h"

bool SkyCubeNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
{
	Node::LoadAttributesFromXML(xmlData);

	if (ParsingAux::AttributeExists(xmlData, "cubemap"))
	{
		m_cubemap = AssetManager::GetOrCreate<TexturePod>(xmlData->Attribute("cubemap"));
	}
	else return false;

	return true;
}

std::string SkyCubeNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--SkyCube " + Node::ToString(verbose, depth);
}
