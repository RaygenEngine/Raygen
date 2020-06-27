#pragma once
#include "assets/AssetPod.h"
#include "assets/pods/MaterialInstance.h"

struct SkinnedVertex {
	glm::vec3 position{};
	glm::vec3 normal{};
	glm::vec3 tangent{};
	glm::vec3 bitangent{};
	glm::vec2 uv{};
	glm::u16vec4 joint{};
	glm::vec4 weight{};
};

struct SkinnedGeometryGroup {
	std::vector<uint32> indices{};
	std::vector<SkinnedVertex> vertices{};

	uint32 materialIndex{ 0u };
};

struct SkinnedMesh : public AssetPod {
	REFLECTED_POD(SkinnedMesh)
	{
		REFLECT_ICON(FA_SKULL);
		REFLECT_VAR(materials);
	}


	std::vector<glm::mat4> jointMatrices{};
	std::vector<uint32> parentJoint{};

	std::vector<SkinnedGeometryGroup> geometryGroups{};

	std::vector<PodHandle<MaterialInstance>> materials{};
};
