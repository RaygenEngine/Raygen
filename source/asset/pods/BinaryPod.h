#pragma once

#include "asset/AssetPod.h"
#include "reflection/GenMacros.h"

struct BinaryPod : public AssetPod {
	REFLECTED_POD(BinaryPod) {}

	static void Load(PodEntry* entry, BinaryPod* pod, const uri::Uri& path);

	std::vector<byte> data;
};
