#pragma once

#include "asset/AssetPod.h"
#include "asset/pods/StringPod.h"

struct ShaderPod : public AssetPod {
	REFLECTED_POD(ShaderPod)
	{
		REFLECT_VAR(vertex);
		REFLECT_VAR(fragment);
	}
	static bool Load(ShaderPod* pod, const uri::Uri& path);

	PodHandle<StringPod> vertex;
	PodHandle<StringPod> fragment;
};
