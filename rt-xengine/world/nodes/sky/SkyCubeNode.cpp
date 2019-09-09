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

	if (ParsingAux::AttributeExists(xmlData, "cube_map"))
	{		
		const auto generalPath = xmlData->Attribute("cube_map");
		const auto name = PathSystem::GetPathWithoutExtension(generalPath);
		const auto extension = PathSystem::GetExtension(generalPath);

		m_cubeMap.SetName(name);
		
		// if any of faces fail
		if (!m_cubeMap.LoadFaceTexture(CMF_RIGHT, name + "_rt" + extension) ||
			!m_cubeMap.LoadFaceTexture(CMF_LEFT, name + "_lf" + extension)  ||
			!m_cubeMap.LoadFaceTexture(CMF_UP, name + "_up" + extension)    ||
			!m_cubeMap.LoadFaceTexture(CMF_DOWN, name + "_dn" + extension)  ||
			!m_cubeMap.LoadFaceTexture(CMF_FRONT, name + "_ft" + extension) ||
			!m_cubeMap.LoadFaceTexture(CMF_BACK, name + "_bk" + extension))
			return false;
	}

	return true;
}

std::string SkyCubeNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--SkyCube " + Node::ToString(verbose, depth);
}
