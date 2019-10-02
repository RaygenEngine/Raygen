#pragma once

#include "core/reflection/GenMacros.h"
#include "asset/AssetPod.h"
#include "asset/pods/MaterialPod.h"

struct GeometryGroup
{
	std::vector<uint32> indices;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec4> tangents;
	std::vector<glm::vec3> bitangents;
	std::vector<glm::vec2> textCoords0;
	std::vector<glm::vec2> textCoords1;

	GeometryMode mode{ GeometryMode::TRIANGLES };

	uint32 materialIndex;
};

struct Mesh
{
	std::vector<GeometryGroup> geometryGroups;
};

struct ModelPod : public AssetPod
{
	REFLECTED_POD(ModelPod)
	{
		REFLECT_VAR(materials);
	}
	static bool Load(ModelPod* pod, const fs::path& path);

	std::vector<Mesh> meshes;

	std::vector<PodHandle<MaterialPod>> materials;
};

