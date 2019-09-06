#include "pch.h"

#include "world/nodes/geometry/TriangleModelGeometryNode.h"
#include "world/World.h"
#include "assets/AssetManager.h"
#include "assets/other/xml/ParsingAux.h"
#include "assets/AssetManager.h"
#include "system/Engine.h"

TriangleModelGeometryNode::TriangleModelGeometryNode(Node* parent)
	: Node(parent)
{
}

std::string TriangleModelGeometryNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--TMgeometry " + Node::ToString(verbose, depth);
}

bool TriangleModelGeometryNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
{
	Node::LoadAttributesFromXML(xmlData);

	std::string type;
	ParsingAux::ReadStringAttribute(xmlData, "type", type);
		
	// default geom is static
	auto modelGeomType = GeometryUsage::STATIC;
	if(!type.empty() && Core::CaseInsensitiveCompare(type, "dynamic"))
		modelGeomType = GeometryUsage::DYNAMIC;
		
	m_model = Engine::GetAssetManager()->LoadModelAsset(xmlData->Attribute("file"), modelGeomType);

	return static_cast<bool>(m_model.get());
}

