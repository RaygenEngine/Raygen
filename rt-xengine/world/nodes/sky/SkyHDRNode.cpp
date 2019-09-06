#include "pch.h"

#include "world/nodes/sky/SkyHDRNode.h"
#include "assets/other/xml/ParsingAux.h"
#include "assets/AssetManager.h"


SkyHDRNode::SkyHDRNode(Node* parent)
	: Node(parent)
{
}

bool SkyHDRNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
{
	Node::LoadAttributesFromXML(xmlData);

	if (ParsingAux::AttributeExists(xmlData, "hdr_texture"))
	{
		m_hdrTexture = GetAssetManager()->LoadTextureAsset(xmlData->Attribute("hdr_texture"), DynamicRange::HIGH);

		if (!m_hdrTexture)
			return false;
	}

	return true;
}

std::string SkyHDRNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--SkyHDR " + Node::ToString(verbose, depth);
}
