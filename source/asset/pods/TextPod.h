#pragma once

#include "system/reflection/Reflection.h"
#include "asset/AssetPod.h"

struct TextPod : DeletableAssetPod
{
	STATIC_REFLECTOR(TextPod)
	{
		S_REFLECT_VAR(data, PropertyFlags::Multiline);
	}

	static bool Load(TextPod* pod, const fs::path& path);

	std::string data;
};