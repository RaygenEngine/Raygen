#pragma once

#include "system/reflection/Reflector.h"
#include "asset/AssetPod.h"
#include "asset/pods/TexturePod.h"

struct CubemapPod : AssetPod
{
	TexturePod* sides[CMF_COUNT]{ nullptr };
};

