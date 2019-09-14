#pragma once

#include "asset/Asset.h"
#include "asset/pods/MaterialPod.h"

struct GeometryGroupPod
{
	std::vector<uint32> indices;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec4> tangents;
	std::vector<glm::vec3> bitangents;
	std::vector<glm::vec2> textCoords0;
	std::vector<glm::vec2> textCoords1;

	GeometryMode mode{ GeometryMode::TRIANGLES };

	MaterialPod material;
};

struct MeshPod
{
	std::vector<GeometryGroupPod*> geometryGroups;
};

class ModelAsset : public Asset
{
public:

private:
	std::vector<MeshPod*> m_meshes;
	

	bool LoadFromGltfImpl();
public:
	
	ModelAsset(const fs::path& path)
		: Asset(path) {}

	[[nodiscard]] std::vector<MeshPod*>& GetMeshes() { return m_meshes; }
	
protected:
	bool Load() override;
	void Unload() override;
};
