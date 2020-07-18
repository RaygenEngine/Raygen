#pragma once
#include "assets/AssetPod.h"
#include "assets/shared/GeometryShared.h"

struct Mesh : AssetPod {

	REFLECTED_POD(Mesh)
	{
		REFLECT_ICON(FA_CUBE);
		REFLECT_VAR(materials);
	}

	std::vector<GeometrySlot> geometrySlots{};

	std::vector<PodHandle<MaterialInstance>> materials{};
};
