#include "pch.h"

#include "assets/model/Mesh.h"
#include "tinygltf/tiny_gltf.h"

bool Mesh::Load(const tinygltf::Model& modelData, const tinygltf::Mesh& meshData, const glm::mat4& transformMat)
{
	// primitives
	auto i = 0;
	for (auto& gltfPrimitive : meshData.primitives)
	{
		const auto name = "geom_group" + std::to_string(i++);
		std::unique_ptr<GeometryGroup> geomGroup = std::make_unique<GeometryGroup>();

		
		if(!geomGroup->Load(modelData, gltfPrimitive, transformMat))
		{
			LOG_ERROR("Failed to load geometry group, {}", geomGroup);
			return false;
		}

		m_geometryGroups.emplace_back(std::move(geomGroup));
	}

	// TODO: weights

	return true;
}

