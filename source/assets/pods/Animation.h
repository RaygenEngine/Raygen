#pragma once
#include "assets/AssetPod.h"
#include "reflection/GenMacros.h"
#include "assets/pods/SkinnedMesh.h"

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
	std::vector<float> inputs{};
	// WIP: handle those depending on the animation path of the channel targeting this sampler
	// translation -> vec3, rotation -> vec4, scale -> vec3, weights -> float
	std::vector<glm::vec4> outputs{};
	InterpolationMethod interpolation{};


	glm::vec3 GetOutputAsVec3(size_t i) const { return outputs[i].xyz; }
	glm::quat GetOutputAsQuat(size_t i) const
	{
		glm::quat q;
		q.x = outputs[i].x;
		q.y = outputs[i].y;
		q.z = outputs[i].z;
		q.w = outputs[i].w;
		return glm::normalize(q);
	}
};

struct AnimationChannel {
	int32 samplerIndex{ -1 };
	AnimationPath path{};
	int32 targetJoint{ -1 };
};

struct Animation : public AssetPod {
	REFLECTED_POD(Animation) { REFLECT_ICON(FA_PLAY); }

	std::vector<AnimationChannel> channels{};
	std::vector<AnimationSampler> samplers{};

	float time;
	int32 jointCount;
};
