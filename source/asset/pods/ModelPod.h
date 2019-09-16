#pragma once

#include "system/reflection/Reflector.h"
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

	PodHandle<MaterialPod> material;
};

struct Mesh
{
	std::vector<GeometryGroup> geometryGroups;
};

struct ModelPod : DeletableAssetPod
{
	STATIC_REFLECTOR(ModelPod)
	{
		S_REFLECT_VAR(lastMaterial);
	}
	static bool Load(ModelPod* pod, const fs::path& path);

	std::vector<Mesh> meshes;

	PodHandle<MaterialPod> lastMaterial;
};

