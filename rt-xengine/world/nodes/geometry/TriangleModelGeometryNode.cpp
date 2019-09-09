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
	REFLECT_VAR(m_model);
}

std::string TriangleModelGeometryNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--TMgeometry " + Node::ToString(verbose, depth);
}

bool TriangleModelGeometryNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
{
	Node::LoadAttributesFromXML(xmlData);

	/*	std::string type;
		Assets::ReadStringAttribute(xmlData, "type", type); 
		
	// default geom is static
	auto modelGeomType = GeometryUsage::STATIC;
	if(!type.empty() && utl::CaseInsensitiveCompare(type, "dynamic"))
		modelGeomType = GeometryUsage::DYNAMIC;
		
	m_model = Engine::GetAssetManager()->LoadModelAsset(xmlData->Attribute("file"), modelGeomType);

		return static_cast<bool>(m_model.get());
		*/
	return true;
}

