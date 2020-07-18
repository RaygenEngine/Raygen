#pragma once
#include "assets/AssetPod.h"
#include "assets/shared/AnimationShared.h"
#include "reflection/GenMacros.h"

struct Animation : public AssetPod {
	REFLECTED_POD(Animation) { REFLECT_ICON(FA_PLAY); }

	std::vector<AnimationChannel> channels{};
	std::vector<AnimationSampler> samplers{};

	float time;
	int32 jointCount;
};
