#include "pch.h"

#include "world/nodes/sky/SkyHDRNode.h"
#include "assets/other/xml/ParsingAux.h"
#include "assets/AssetManager.h"
#include "system/Engine.h"


SkyHDRNode::SkyHDRNode(Node* parent)
	: Node(parent)
{
}

bool SkyHDRNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
{
	Node::LoadAttributesFromXML(xmlData);

	if (ParsingAux::AttributeExists(xmlData, "hdr_texture"))
	{
		auto finalPath = Engine::GetAssetManager()->m_pathSystem.SearchAsset(xmlData->Attribute("hdr_texture"));
		m_hdrTexture = Engine::GetAssetManager()->MaybeGenerateAsset<TextureAsset>(finalPath);
		if (!Engine::GetAssetManager()->Load(m_hdrTexture) || !m_hdrTexture->IsHdr())
			return false;
	}
	else return false;

	return true;
}

std::string SkyHDRNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--SkyHDR " + Node::ToString(verbose, depth);
}
