#pragma once

#include "asset/AssetPod.h"

struct StringPod : public AssetPod
{
	REFLECTED_POD(StringPod)
	{
		REFLECT_VAR(data, PropertyFlags::Multiline);
	}

	static bool Load(StringPod* pod, const fs::path& path);

	std::string data;
};