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

enum class GeometryUsage
{
	DYNAMIC,
	STATIC,
	STREAM
};

struct GeometryGroup {
	std::vector<uint32> indices{};
	std::vector<VertexData> vertices{};

	GeometryMode mode{ GeometryMode::TRIANGLES };
	GeometryUsage usage{ GeometryUsage::STATIC };
	uint32 materialIndex{ 0u };
};

struct Mesh {
	std::vector<GeometryGroup> geometryGroups{};
};

struct ModelPod : public AssetPod {
	REFLECTED_POD(ModelPod) { REFLECT_VAR(materials); }

	static void Load(ModelPod* pod, const uri::Uri& path);

	std::vector<Mesh> meshes{};

	Box bbox;

	std::vector<PodHandle<MaterialPod>> materials{};
};
