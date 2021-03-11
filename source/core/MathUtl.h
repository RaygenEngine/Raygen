#pragma once

//#define GLM_FORCE_MESSAGES
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_CXX2A
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_XYZW_ONLY

// WIP: Remove glm math
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

// XMGLOBALCONST XMVECTORF32 g_XMIdentityR0 = { { { 1.0f, 0.0f, 0.0f, 0.0f } } }; // engine right
// XMGLOBALCONST XMVECTORF32 g_XMIdentityR1 = { { { 0.0f, 1.0f, 0.0f, 0.0f } } }; // engine up
// XMGLOBALCONST XMVECTORF32 g_XMNegIdentityR2 = { { { 0.0f, 0.0f, -1.0f, 0.0f } } }; // engine front

struct TransformCache {

	TransformCache()
	{
		pData = (AlignedData*)_aligned_malloc(sizeof(AlignedData), 16);
		pData->translation = XMVectorZero();
		pData->scale = XMVectorZero();
		pData->transform = XMMatrixIdentity();
		pData->orientation = XMQuaternionIdentity();
	}
	~TransformCache() { _aligned_free(pData); }

	// This data must be aligned otherwise the SSE intrinsics fail
	// and throw exceptions.
	__declspec(align(16)) struct AlignedData {
		XMVECTOR translation;
		XMVECTOR orientation;
		XMVECTOR scale;

		XMMATRIX transform;
	};
	AlignedData* pData;

	[[nodiscard]] XMVECTOR up() const { return XMVector3Rotate(g_XMIdentityR1, pData->orientation); }
	[[nodiscard]] XMVECTOR front() const { return XMVector3Rotate(g_XMNegIdentityR2, pData->orientation); }
	[[nodiscard]] XMVECTOR right() const { return XMVector3Rotate(g_XMIdentityR0, pData->orientation); }
	[[nodiscard]] XMVECTOR translation() const { return pData->translation; }
	[[nodiscard]] XMVECTOR orientation() const { return pData->orientation; }
	[[nodiscard]] XMVECTOR scale() const { return pData->scale; }
	[[nodiscard]] XMMATRIX transform() const { return pData->transform; }


	void XM_CALLCONV set_translation(FXMVECTOR translation) { pData->translation = translation; }
	void XM_CALLCONV set_orientation(FXMVECTOR orientation) { pData->orientation = orientation; }
	void XM_CALLCONV set_scale(FXMVECTOR scale) { pData->scale = scale; }
	void XM_CALLCONV set_transform(FXMMATRIX transform) { pData->transform = transform; }

	// don't use glm types here - should not get things too tangled
	void set_translation(float x, float y, float z) { pData->translation = XMVectorSet(x, y, z, 1.0); }
	void set_orientation(float x, float y, float z, float w) { pData->orientation = XMVectorSet(x, y, z, w); }
	void set_scale(float x, float y, float z) { pData->scale = XMVectorSet(x, y, z, 0.0); }

	//  NEW::
	// pitch, yaw, roll, in degrees
	//[[nodiscard]] glm::vec3 pyr() const { return glm::degrees(glm::eulerAngles(orientation)); }
	//[[nodiscard]] float pitch() const { return glm::degrees(glm::pitch(orientation)); }
	// [[nodiscard]] float yaw() const { return glm::degrees(glm::yaw(orientation)); }
	//[[nodiscard]] float roll() const { return glm::degrees(glm::roll(orientation)); }

	// TODO: Move compose/decompose from BasicComponent.cpp
	// Updates transform from TRS
	// WIP: 2nd param of mat func
	void compose()
	{
		pData->transform = XMMatrixAffineTransformation(pData->scale, {}, pData->orientation, pData->translation);
	}
	// Updates TRS from transform
	void decompose() { XMMatrixDecompose(&pData->scale, &pData->orientation, &pData->translation, pData->transform); }
};
