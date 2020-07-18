#pragma once
#include "assets/AssetPod.h"
#include "assets/shared/GeometryShared.h"

struct SkinnedMesh : public AssetPod {
	[[nodiscard]] static bool IsRootParent(int32 jointIndex) noexcept
	{
		return jointIndex == Joint::c_rootParentJointIndex;
	}

	[[nodiscard]] bool IsRootJoint(int32 jointIndex) const noexcept
	{
		return IsRootParent(joints[jointIndex].parentJoint);
	}

	REFLECTED_POD(SkinnedMesh)
	{
		REFLECT_ICON(FA_SKULL);
		REFLECT_VAR(materials);
	}

	std::vector<Joint> joints;
	std::vector<SkinnedGeometrySlot> skinnedGeometrySlots{};
	std::vector<PodHandle<MaterialInstance>> materials{};
};
