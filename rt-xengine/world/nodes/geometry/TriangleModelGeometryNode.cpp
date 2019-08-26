#include "pch.h"

#include "TriangleModelGeometryNode.h"

namespace World
{
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
		Assets::ReadStringAttribute(xmlData, "type", type); 
		
		// default geom is static
		auto modelGeomType = GU_STATIC;
		if(!type.empty() && Core::CaseInsensitiveCompare(type, "dynamic"))
			modelGeomType = GU_DYNAMIC;
		
		m_model = GetDiskAssetManager()->LoadFileAsset<Assets::Model>(xmlData->Attribute("file"), GetWorld()->GetAssetLoadPathHint());

		// missing model
		if (!m_model)
			return false;

		// TODO, set
		//m_model->SetType(modelGeomType);

		return true;
	}
}
