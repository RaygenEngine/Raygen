#pragma once

namespace utl {
template<typename T>
inline bool EqualsZero(T value)
{
	return glm::epsilonEqual(value, 0.f, glm::epsilon<T>());
}

template<typename T>
inline bool EqualsValue(T input, T value)
{
	return glm::epsilonEqual(input, value, glm::epsilon<T>());
}

inline glm::mat4 GetTransformMat(const glm::vec3& translation, const glm::quat& orientation, const glm::vec3& scale)
{
	glm::mat4 transMat(1.f);
	// T
	transMat = glm::translate(transMat, translation);
	// T * R * S
	transMat = glm::scale(transMat * glm::mat4_cast(orientation), scale);

	return transMat;
}

inline glm::quat GetOrientationFromLookAtAndPosition(const glm::vec3& lookat, const glm::vec3& position)
{
	const auto dlp = glm::distance(lookat, position);
	// if can't get orientation return unit
	if (EqualsZero(dlp))
		return glm::quat{ 1.f, 0.f, 0.f, 0.f };

	const auto direction = glm::normalize(lookat - position);

	// if parallel use right as up
	const auto ddu = abs(glm::dot(direction, glm::vec3(0.f, 1.f, 0.f)));
	if (EqualsValue(ddu, 1.f))
		return glm::normalize(glm::quatLookAt(direction, glm::vec3(1.f, 0.f, 0.f)));
	else
		return glm::normalize(glm::quatLookAt(direction, glm::vec3(0.f, 1.f, 0.f)));
}

inline glm::mat4 GetProjectionMatrix(
	float topTan, float bottomTan, float rightTan, float leftTan, float _near, float _far)
{
	const auto top = _near * topTan;
	const auto bottom = -_near * bottomTan;
	const auto right = _near * rightTan;
	const auto left = -_near * leftTan;

	return glm::frustum(left, right, bottom, top, _near, _far);
}

// TODO
/*inline glm::mat4 GetProjectionMatrix(const XFov& xfov, float _near, float _far)
{
	const auto top = _near * xfov.topTan;
	const auto bottom = -_near * xfov.bottomTan;
	const auto right = _near * xfov.rightTan;
	const auto left = -_near * xfov.leftTan;

	return glm::frustum(left, right, bottom, top, _near, _far);
}*/
} // namespace utl
