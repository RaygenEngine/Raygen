#pragma once

//#define GLM_FORCE_MESSAGES
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_CXX2A
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

constexpr glm::vec3 engineSpaceUp{ 0.f, 1.f, 0.f };
constexpr glm::vec3 engineSpaceForward{ 0.f, 0.f, -1.f };
constexpr glm::vec3 engineSpaceRight{ 1.f, 0.f, 0.f };

struct TransformCache {
	glm::vec3 position{};
	glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
	glm::quat orientation{ glm::identity<glm::quat>() };

	glm::mat4 transform{ glm::identity<glm::mat4>() };

	[[nodiscard]] glm::vec3 up() const { return orientation * engineSpaceUp; }
	[[nodiscard]] glm::vec3 forward() const { return orientation * engineSpaceForward; };
	[[nodiscard]] glm::vec3 right() const { return orientation * engineSpaceRight; };
	// pitch, yaw, roll, in degrees
	[[nodiscard]] glm::vec3 pyr() const { return glm::degrees(glm::eulerAngles(orientation)); }
	[[nodiscard]] void setPyr(glm::vec3 pyr) { orientation = glm::quat(glm::radians(pyr)); }

	// TODO: Move compose/decompose from BasicComponent.cpp
	// Updates transform from TRS
	void Compose();
	// Updates TRS from transform
	void Decompose();
};

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

template<typename T>
int32 roundToInt(T number)
{
	return static_cast<int32>(glm::round(number));
}
} // namespace math
