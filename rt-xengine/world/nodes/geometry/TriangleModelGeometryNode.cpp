#include "pch.h"

#include "world/nodes/geometry/TriangleModelGeometryNode.h"
#include "world/World.h"
#include "assets/AssetManager.h"
#include "assets/other/xml/ParsingAux.h"
#include "assets/AssetManager.h"
#include "system/Engine.h"

TriangleModelGeometryNode::TriangleModelGeometryNode(Node* parent)
	: Node(parent),
      m_model(nullptr)
{
	//REFLECT_VAR(m_model);
}

std::string TriangleModelGeometryNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--TMgeometry " + Node::ToString(verbose, depth);
}

bool TriangleModelGeometryNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
{
	Node::LoadAttributesFromXML(xmlData);


	if (ParsingAux::AttributeExists(xmlData, "model"))
	{
		auto finalPath = Engine::GetAssetManager()->m_pathSystem.SearchAsset(xmlData->Attribute("model"));
		m_model = Engine::GetAssetManager()->MaybeGenerateAsset<Model>(finalPath / fs::path("model"));
		if (!Engine::GetAssetManager()->Load(m_model))
			return false;
	}
	else return false;

	return true;
}

