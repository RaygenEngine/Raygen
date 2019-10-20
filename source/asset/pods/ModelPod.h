#pragma once

#include "asset/AssetPod.h"
#include "asset/pods/MaterialPod.h"

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

	Box bbox{ glm::vec3(.5f), glm::vec3(-.5f) };

	std::vector<PodHandle<MaterialPod>> materials{};
};
