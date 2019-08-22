#include "pch.h"

#include "SkyCubeNode.h"
#include "world/World.h"

namespace World
{
	SkyCubeNode::SkyCubeNode(Node* parent)
		: Node(parent)
	{
	}

	bool SkyCubeNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
	{
		Node::LoadAttributesFromXML(xmlData);

		if (Assets::AttributeExists(xmlData, "cube_map"))
		{
			m_cubeMap = GetDiskAssetManager()->LoadCubeMapAsset(xmlData->Attribute("cube_map"), Assets::DR_LOW, false);

			if (!m_cubeMap)
				return false;
		}

		return true;
	}

	std::string SkyCubeNode::ToString(bool verbose, uint depth) const
	{
		return std::string("    ") * depth + "|--SkyCube " + Node::ToString(verbose, depth);
	}
}
