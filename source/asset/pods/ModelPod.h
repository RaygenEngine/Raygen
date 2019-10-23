#pragma once

#include "asset/AssetPod.h"
#include "asset/pods/MaterialPod.h"
#include "core/MathAux.h"

enum class GeometryMode
{
	POINTS,
	LINE,
	LINE_LOOP,
	LINE_STRIP,
	TRIANGLES,
	TRIANGLE_STRIP,
	TRIANGLE_FAN
};

struct VertexData {
	glm::vec3 position{};
	glm::vec3 normal{};
	glm::vec3 tangent{};
	glm::vec3 bitangent{};
	glm::vec2 textCoord0{};
	glm::vec2 textCoord1{};
};

struct GeometryGroup {
	std::vector<uint32> indices{};
	std::vector<VertexData> vertices{};

	GeometryMode mode{ GeometryMode::TRIANGLES };
	uint32 materialIndex{ 0u };
};

struct Mesh {
	std::vector<GeometryGroup> geometryGroups{};
};

struct ModelPod : public AssetPod {

	REFLECTED_POD(ModelPod) { REFLECT_VAR(materials); }

	static void Load(ModelPod* pod, const uri::Uri& path);

	std::vector<Mesh> meshes{};

	math::AABB bbox{ glm::vec3(.5f), glm::vec3(-.5f) };

	std::vector<PodHandle<MaterialPod>> materials{};
};
