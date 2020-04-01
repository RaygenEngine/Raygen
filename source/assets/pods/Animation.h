#pragma once
#include "assets/AssetPod.h"
#include "reflection/GenMacros.h"

enum class AnimationPath
{
	Translation,
	Rotation,
	Scale,
	Weights
};

enum class InterpolationMethod
{
	Linear,
	Step,
	Cubicspline
};

struct AnimationSampler {
	float input;
	float output;
	InterpolationMethod im{};
};

struct Channel {
	AnimationSampler sampler{};
	AnimationPath path{};
};


struct Animation : public AssetPod {
	REFLECTED_POD(Animation) { REFLECT_ICON(FA_PLAY); }

	std::vector<Channel> channels;
};
