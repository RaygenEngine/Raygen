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
	static constexpr int32 c_rootParentJointIndex = -1;

	[[nodiscard]] static bool IsRootParent(int32 jointIndex) noexcept { return jointIndex == c_rootParentJointIndex; }

	[[nodiscard]] bool IsRootJoint(int32 jointIndex) const noexcept
	{
		return IsRootParent(joints[jointIndex].parentJoint);
	}

	struct Joint {
		glm::mat4 inverseBindMatrix;
		glm::mat4 localTransform;
		int32 parentJoint;
		int32 index;

		std::string name;

		[[nodiscard]] Joint& GetParent(SkinnedMesh* pod) const { return pod->joints[parentJoint]; };
		[[nodiscard]] bool IsRoot() const { return parentJoint == c_rootParentJointIndex; }
	};

	REFLECTED_POD(SkinnedMesh)
	{
		REFLECT_ICON(FA_SKULL);
		REFLECT_VAR(materials);
	}

	std::vector<Joint> joints;


	std::vector<SkinnedGeometrySlot> skinnedGeometrySlots{};

	std::vector<PodHandle<MaterialInstance>> materials{};
};
