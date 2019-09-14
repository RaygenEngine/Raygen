#pragma once

#include "system/reflection/Reflector.h"
#include "asset/AssetPod.h"
#include "asset/pods/TextPod.h"

struct ShaderPod : AssetPod
{
	TextPod* vertex{ nullptr };
	TextPod* fragment{ nullptr };
};

