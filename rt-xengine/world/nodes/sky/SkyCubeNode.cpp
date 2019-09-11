#include "pch.h"

#include "world/nodes/sky/SkyCubeNode.h"
#include "assets/other/xml/ParsingAux.h"
#include "assets/AssetManager.h"

bool SkyCubeNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
{
	Node::LoadAttributesFromXML(xmlData);

	if (ParsingAux::AttributeExists(xmlData, "cube_map"))
	{
		auto finalPath = Engine::GetAssetManager()->m_pathSystem.SearchAsset(xmlData->Attribute("cube_map"));
		m_cubeMap = Engine::GetAssetManager()->MaybeGenerateAsset<CubeMapAsset>(finalPath);
		if (!Engine::GetAssetManager()->Load(m_cubeMap))
			return false;
	}
	else return false;

	return true;
}

std::string SkyCubeNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--SkyCube " + Node::ToString(verbose, depth);
}
