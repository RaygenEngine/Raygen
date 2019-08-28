#include "pch.h"

#include "assets/model/Mesh.h"
#include "tinygltf/tiny_gltf.h"

namespace Assets
{
	Mesh::Mesh(DiskAsset* pAsset, const std::string& name)
		: DiskAssetPart(pAsset, name)
	{
	}

	void Mesh::LoadFromGltfData(const tinygltf::Model& modelData, const tinygltf::Mesh& meshData)
	{
		// primitives
		auto i = 0;
		for (auto& gltfPrimitive : meshData.primitives)
		{
			const auto name = "geom_group" + std::to_string(i++);
			GeometryGroup geometryGroup{this, name };
			geometryGroup.LoadFromGltfData(modelData, gltfPrimitive);

			m_geometryGroups.emplace_back(geometryGroup);
		}

		// TODO: weights
	}
}
