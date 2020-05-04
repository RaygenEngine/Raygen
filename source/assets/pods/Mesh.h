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

// Transition note: (Former GeometryGroup)
// This "slot" now batches all vertices per material during importing.
// GeometrySlots now is a parallel array to the materials array. (ie: geometrySlots[2] material == materials[2])
struct GeometrySlot {
	std::vector<uint32> indices{};
	std::vector<Vertex> vertices{};
};

struct Mesh : AssetPod {

	REFLECTED_POD(Mesh)
	{
		REFLECT_ICON(FA_CUBE);
		REFLECT_VAR(materials);
	}

	std::vector<GeometrySlot> geometrySlots{};

	std::vector<PodHandle<Material>> materials{};
};
