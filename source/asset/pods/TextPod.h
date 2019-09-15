#pragma once

#include "system/reflection/Reflector.h"
#include "asset/AssetPod.h"

struct TextPod : DeletableAssetPod
{
	static bool Load(TextPod* pod, const fs::path& path);

	std::string data;
};