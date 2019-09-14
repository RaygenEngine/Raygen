#include "pch.h"

#include "world/nodes/sky/SkyHDRNode.h"
#include "asset/util/ParsingAux.h"
#include "asset/AssetManager.h"
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
		m_hdrData = Engine::GetAssetManager()->RequestSearchAsset<ImageAsset>(xmlData->Attribute("hdr_texture"))->GetPod();
	}
	else return false;

	return true;
}

std::string SkyHDRNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--SkyHDR " + Node::ToString(verbose, depth);
}
