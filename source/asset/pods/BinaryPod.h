#pragma once

#include "asset/AssetPod.h"

struct BinaryPod : public AssetPod {

	static void Load(BinaryPod* pod, const uri::Uri& path);

	std::vector<char> data;
};
