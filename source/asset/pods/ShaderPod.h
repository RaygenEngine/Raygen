#pragma once

#include "asset/AssetPod.h"
#include "asset/pods/StringPod.h"

struct ShaderPod : public AssetPod {
	REFLECTED_POD(ShaderPod)
	{
		REFLECT_VAR(vertex);
		REFLECT_VAR(geometry);
		REFLECT_VAR(fragment);
	}
	static void Load(ShaderPod* pod, const uri::Uri& path);

	// WIP: use vector and decide in shader?
	PodHandle<StringPod> vertex;
	PodHandle<StringPod> geometry;
	PodHandle<StringPod> fragment;
};
