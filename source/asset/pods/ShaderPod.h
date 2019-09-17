#pragma once

#include "system/reflection/Reflector.h"
#include "asset/AssetPod.h"
#include "asset/pods/StringPod.h"

struct ShaderPod : AssetPod
{
	static bool Load(ShaderPod* pod, const fs::path& path);

	PodHandle<StringPod> vertex;
	PodHandle<StringPod> fragment;
};

