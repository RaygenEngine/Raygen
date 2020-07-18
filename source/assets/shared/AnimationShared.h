#pragma once

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
	std::vector<glm::vec4> outputs{};
	InterpolationMethod interpolation{};


	glm::vec3 GetOutputAsVec3(size_t i) const { return glm::vec3(outputs[i]); }
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
