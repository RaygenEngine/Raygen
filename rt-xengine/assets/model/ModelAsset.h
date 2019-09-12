#pragma once

#include "assets/Asset.h"
#include "assets/model/MaterialAsset.h"

class ModelAsset : public Asset
{
public:
	struct Mesh
	{
		struct GeometryGroup
		{
			std::vector<uint32> indices;

			std::vector<glm::vec3> positions;
			std::vector<glm::vec3> normals;
			std::vector<glm::vec4> tangents;
			std::vector<glm::vec3> bitangents;
			std::vector<glm::vec2> textCoords0;
			std::vector<glm::vec2> textCoords1;

			// TODO joints/weights

			GeometryMode mode{ GeometryMode::TRIANGLES };

			MaterialPod* material{ nullptr };
		};

		std::vector<GeometryGroup> geometryGroups;

		// TODO anims
	};
private:

	std::vector<Mesh> m_meshes;
	
public:
	
	ModelAsset(const fs::path& path)
		: Asset(path) {}

	[[nodiscard]] std::vector<Mesh> GetMeshes() const { return m_meshes; }
	
protected:
	bool Load() override;
	void Unload() override;
};
