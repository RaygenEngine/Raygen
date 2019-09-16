#pragma once

#include "system/reflection/Reflector.h"
#include "asset/AssetPod.h"
#include "asset/pods/TexturePod.h"

struct CubemapPod : AssetPod
{
	STATIC_REFLECTOR(CubemapPod)
	{
		reflector.AutoAddProperty<PodHandle<TexturePod>>("sides[0]", offsetof(__ThisType, sides[0]));
		reflector.AutoAddProperty<PodHandle<TexturePod>>("sides[1]", offsetof(__ThisType, sides[1]));
		reflector.AutoAddProperty<PodHandle<TexturePod>>("sides[2]", offsetof(__ThisType, sides[2]));
		
	}
	static bool Load(CubemapPod* pod, const fs::path& path);

	PodHandle<TexturePod> sides[CMF_COUNT];
};

