#pragma once

#include "asset/AssetPod.h"
#include "reflection/GenMacros.h"
struct StringPod : public AssetPod {
	REFLECTED_POD(StringPod) { REFLECT_VAR(data, PropertyFlags::Multiline); }

	static void Load(PodEntry* entry, StringPod* pod, const uri::Uri& path);

	std::string data;
};
