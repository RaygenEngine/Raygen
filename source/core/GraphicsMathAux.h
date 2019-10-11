#pragma once

namespace math {
template<typename T>
inline bool EpsilonEqualsValue(T input, T value)
{
	return glm::epsilonEqual(input, value, glm::epsilon<T>());
}

template<typename T>
inline bool EpsilonEqualsZero(T input)
{
	return EpsilonEqualsValue(input, T{ 0.f });
}

inline glm::mat4 TransformMatrixFromTOS(glm::vec3 scale, glm::quat orientation, glm::vec3 translation)
{
	glm::mat4 S = glm::scale(scale);
	glm::mat4 R = glm::toMat4(orientation);
	glm::mat4 T = glm::translate(translation);

	return T * R * S;
}

inline glm::mat4 TransformMatrixFromTRS(glm::vec3 scale, glm::vec3 raxis, float rads, glm::vec3 translation)
{
	glm::mat4 S = glm::scale(scale);
	glm::mat4 R = glm::rotate(rads, raxis);
	glm::mat4 T = glm::translate(translation);

	return T * R * S;
}

inline glm::quat OrientationFromLookatAndPosition(glm::vec3 lookat, glm::vec3 position)
{
	const auto dlp = glm::distance(lookat, position);
	// if can't get orientation return unit
	if (EpsilonEqualsZero(dlp)) {
		return glm::quat{ 1.f, 0.f, 0.f, 0.f };
	}

	const auto direction = glm::normalize(lookat - position);

	// if parallel use right as up
	const auto ddu = abs(glm::dot(direction, glm::vec3(0.f, 1.f, 0.f)));
	if (EpsilonEqualsValue(ddu, 1.f)) {
		return glm::normalize(glm::quatLookAt(direction, glm::vec3(1.f, 0.f, 0.f)));
	}

	return glm::normalize(glm::quatLookAt(direction, glm::vec3(0.f, 1.f, 0.f)));
}
} // namespace math
