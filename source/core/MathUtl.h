#pragma once

//#define GLM_FORCE_MESSAGES
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_CXX2A
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_XYZW_ONLY
#define GLM_


#include <glm/glm.hpp>
#include <glm/glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

constexpr glm::vec3 engineSpaceUp{ 0.f, 1.f, 0.f };
constexpr glm::vec3 engineSpaceFront{ 0.f, 0.f, -1.f };
constexpr glm::vec3 engineSpaceRight{ 1.f, 0.f, 0.f };


namespace math {
template<typename T>
bool equals(T input, T value, T epsilon = glm::epsilon<T>())
{
	return glm::epsilonEqual(input, value, glm::epsilon<T>());
}

template<typename T>
bool equalsZero(T input, T epsilon = glm::epsilon<T>())
{
	return equals(input, {}, epsilon);
}

inline glm::mat4 transformMat(glm::vec3 scale, glm::quat orientation, glm::vec3 translation)
{
	const auto s = glm::scale(scale);
	const auto r = glm::toMat4(orientation);
	const auto t = glm::translate(translation);

	return t * r * s;
}

inline glm::mat4 transformMat(glm::vec3 scale, glm::vec3 raxis, float rads, glm::vec3 translation)
{
	const auto s = glm::scale(scale);
	const auto r = glm::rotate(rads, raxis);
	const auto t = glm::translate(translation);

	return t * r * s;
}

inline glm::mat4 transformMat(float uniformScale, glm::vec3 translation)
{
	auto t = glm::translate(translation);
	t[0][0] = uniformScale;
	t[1][1] = uniformScale;
	t[2][2] = uniformScale;
	t[3][3] = 1.f;
	return t;
}

inline glm::mat4 transformMat2(
	glm::vec3 translation, glm::quat orientation = { glm::identity<glm::quat>() }, glm::vec3 scale = { 1.f, 1.f, 1.f })
{
	const auto s = glm::scale(scale);
	const auto r = glm::toMat4(orientation);
	const auto t = glm::translate(translation);

	return t * r * s;
}

inline glm::quat findOrientation(glm::vec3 lookat, glm::vec3 position)
{
	const auto dlp = glm::distance(lookat, position);
	// if can't get orientation return unit
	if (equalsZero(dlp)) {
		return glm::quat{ 1.f, 0.f, 0.f, 0.f };
	}

	const auto direction = glm::normalize(lookat - position);

	// if parallel use right as up
	const auto ddu = abs(glm::dot(direction, glm::vec3(0.f, 1.f, 0.f)));
	if (equals(ddu, 1.f)) {
		return glm::normalize(glm::quatLookAt(direction, glm::vec3(1.f, 0.f, 0.f)));
	}

	return glm::normalize(glm::quatLookAt(direction, glm::vec3(0.f, 1.f, 0.f)));
}

inline glm::quat findLookAt(glm::vec3 source, glm::vec3 target)
{
	const auto dlp = glm::distance(target, source);
	// if can't get orientation return unit
	if (equalsZero(dlp)) {
		return glm::quat{ 1.f, 0.f, 0.f, 0.f };
	}

	const auto direction = glm::normalize(target - source);

	// if parallel use right as up
	const auto ddu = abs(glm::dot(direction, glm::vec3(0.f, 1.f, 0.f)));
	if (equals(ddu, 1.f)) {
		return glm::normalize(glm::quatLookAt(direction, glm::vec3(1.f, 0.f, 0.f)));
	}

	return glm::normalize(glm::quatLookAt(direction, glm::vec3(0.f, 1.f, 0.f)));
}

// "decompose" aka return the proper column
inline glm::vec3 decomposePos(const glm::mat4& matrix)
{
	return glm::vec3(matrix[3]);
}

template<typename T>
int32 roundToInt(T number)
{
	return static_cast<int32>(glm::round(number));
}


template<typename T>
int32 roundToUInt(T number)
{
	return static_cast<uint32>(glm::round(number));
}
} // namespace math

struct TransformCache {
	glm::vec3 position{};
	glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
	glm::quat orientation{ glm::identity<glm::quat>() };

	glm::mat4 transform{ glm::identity<glm::mat4>() };

	[[nodiscard]] glm::vec3 up() const { return orientation * engineSpaceUp; }
	[[nodiscard]] glm::vec3 front() const { return orientation * engineSpaceFront; };
	[[nodiscard]] glm::vec3 right() const { return orientation * engineSpaceRight; };

	// pitch, yaw, roll, in degrees
	[[nodiscard]] glm::vec3 pyr() const { return glm::degrees(glm::eulerAngles(orientation)); }
	[[nodiscard]] float pitch() const { return glm::degrees(glm::pitch(orientation)); }
	[[nodiscard]] float yaw() const { return glm::degrees(glm::yaw(orientation)); }
	[[nodiscard]] float roll() const { return glm::degrees(glm::roll(orientation)); }

	// TODO: Move compose/decompose from BasicComponent.cpp
	// Updates transform from TRS
	void Compose() { transform = math::transformMat(scale, orientation, position); }
	// Updates TRS from transform
	void Decompose()
	{
		glm::vec4 persp{};
		glm::vec3 skew{};

		glm::decompose(transform, scale, orientation, position, skew, persp);
	}
};
