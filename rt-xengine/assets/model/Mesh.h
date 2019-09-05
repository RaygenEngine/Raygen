#pragma once

#include "assets/model/GeometryGroup.h"

namespace tinygltf
{
	class Model;
	struct Mesh;
}

class Mesh
{
	std::vector<std::unique_ptr<GeometryGroup>> m_geometryGroups;

public:

	bool Load(const tinygltf::Model& modelData, const tinygltf::Mesh& meshData, const glm::mat4& transformMat);

	const std::vector<std::unique_ptr<GeometryGroup>>& GetGeometryGroups() const { return m_geometryGroups; }
};
