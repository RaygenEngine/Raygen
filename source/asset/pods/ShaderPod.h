#pragma once

#include "system/reflection/Reflector.h"
#include "asset/AssetPod.h"
#include "asset/pods/TextPod.h"

struct ShaderPod : AssetPod
{
	STATIC_REFLECTOR(ShaderPod)
	{

	}
	static bool Load(ShaderPod* pod, const fs::path& path);

	PodHandle<TextPod> vertex;
	PodHandle<TextPod> fragment;
};

