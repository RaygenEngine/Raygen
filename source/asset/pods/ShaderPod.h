#pragma once

#include "core/reflection/GenMacros.h"
#include "asset/AssetPod.h"
#include "asset/pods/StringPod.h"

struct ShaderPod : public AssetPod
{
	REFLECTED_POD(ShaderPod)
	{
		REFLECT_VAR(vertex);
		REFLECT_VAR(fragment);
	}
	static bool Load(ShaderPod* pod, const fs::path& path);

	PodHandle<StringPod> vertex;
	PodHandle<StringPod> fragment;
};

