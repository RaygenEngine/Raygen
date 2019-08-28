#pragma once

#include "assets/DiskAssetPart.h"
#include "assets/model/GeometryGroup.h"

namespace tinygltf
{
	class Model;
	struct Mesh;
}

namespace Assets
{
	class Mesh : public DiskAssetPart
	{
		std::vector<GeometryGroup> m_geometryGroups;

	public:

		Mesh(DiskAsset* pAsset, const std::string& name);

		void LoadFromGltfData(const tinygltf::Model& modelData, const tinygltf::Mesh& meshData);

		const std::vector<GeometryGroup>& GetGeometryGroups() const { return m_geometryGroups; }

		void ToString(std::ostream& os) const override { os << "asset-type: Mesh, name: " << m_name; }
	};
}
