#pragma once
#include "assets/AssetPod.h"
#include "assets/pods/Material.h"

struct Vertex {
	glm::vec3 position{};
	glm::vec3 normal{};
	glm::vec3 tangent{};
	glm::vec3 bitangent{};
	glm::vec2 uv{};
};

struct GeometryGroup {
	std::vector<uint32> indices{};
	std::vector<Vertex> vertices{};

	uint32 materialIndex{ 0u };
};

struct Mesh : AssetPod {

	REFLECTED_POD(Mesh)
	{
		REFLECT_ICON(FA_CUBE);
		REFLECT_VAR(materials);
	}

	std::vector<GeometryGroup> geometryGroups{};

	std::vector<PodHandle<Material>> materials{};
};
