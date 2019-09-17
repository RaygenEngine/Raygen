#pragma once

#include "system/reflection/Reflector.h"
#include "asset/AssetPod.h"

struct StringPod : DeletableAssetPod
{
	static bool Load(StringPod* pod, const fs::path& path);

	std::string data;
};