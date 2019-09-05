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
		std::vector<std::unique_ptr<GeometryGroup>> m_geometryGroups;

	public:

		Mesh(DiskAsset* pAsset, const std::string& name);

		bool Load(const tinygltf::Model& modelData, const tinygltf::Mesh& meshData, const glm::mat4& transformMat);

		const std::vector<std::unique_ptr<GeometryGroup>>& GetGeometryGroups() const { return m_geometryGroups; }

		void ToString(std::ostream& os) const override { os << "asset-type: Mesh, name: " << m_name; }
	};
}
