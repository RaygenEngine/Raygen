#include "pch.h"

#include "world/nodes/geometry/GeometryNode.h"
#include "world/World.h"
#include "asset/AssetManager.h"
#include "system/Engine.h"
#include "asset/assets/GltfModelAsset.h"
#include "asset/assets/GltfFileAsset.h"
#include "asset/util/ParsingAux.h"

std::string GeometryNode::ToString(bool verbose, uint depth) const
{
	return std::string("    ") * depth + "|--TMgeometry " + Node::ToString(verbose, depth);
}

bool GeometryNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
{
	Node::LoadAttributesFromXML(xmlData);


	if (ParsingAux::AttributeExists(xmlData, "model"))
	{
		fs::path file = xmlData->Attribute("model");
		m_model = AssetManager::GetOrCreate<ModelPod>(file / "#model");
		//if (file.extension().compare(".gltf") == 0)
		//{
		//	auto pParent = Engine::GetAssetManager()->RequestSearchAsset<GltfFileAsset>(file);

		//	auto mPath = pParent->GetUri() / "#model";
		//	// check if gltf
		//	m_model = Engine::GetAssetManager()->RequestSearchAsset<GltfModelAsset>(mPath);
		//}
		// load other types
	}
	else return false;

	return true;
}

