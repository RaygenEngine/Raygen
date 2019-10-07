#pragma once

#include "core/reflection/GenMacros.h"
#include "asset/AssetPod.h"
#include "asset/pods/MaterialPod.h"

struct VertexData
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec4 tangent;
	glm::vec3 bitangent;
	glm::vec2 textCoord0;
	glm::vec2 textCoord1;
};

struct GeometryGroup
{
	std::vector<uint32> indices;
	std::vector<VertexData> vertices;

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
	static bool Load(ModelPod* pod, const uri::Uri& path);

	std::vector<Mesh> meshes;

	std::vector<PodHandle<MaterialPod>> materials;
};

