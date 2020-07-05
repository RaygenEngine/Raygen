#pragma once
#include "assets/AssetPod.h"
#include "assets/pods/MaterialInstance.h"

struct SkinnedVertex {
	glm::vec3 position{};
	glm::vec3 normal{};
	glm::vec3 tangent{};
	glm::vec2 uv{};
	glm::ivec4 joint{};
	glm::vec4 weight{};
};

struct SkinnedGeometrySlot {
	std::vector<uint32> indices{};
	std::vector<SkinnedVertex> vertices{};
};

struct SkinnedMesh : public AssetPod {
	REFLECTED_POD(SkinnedMesh)
	{
		REFLECT_ICON(FA_SKULL);
		REFLECT_VAR(materials);
	}

	std::vector<glm::mat4> jointMatrices{};
	std::vector<uint32> parentJoint{};

	std::vector<SkinnedGeometrySlot> skinnedGeometrySlots{};

	std::vector<PodHandle<MaterialInstance>> materials{};
};
