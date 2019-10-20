#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace math {
template<typename T>
bool EpsilonEqualsValue(T input, T value)
{
	return glm::epsilonEqual(input, value, glm::epsilon<T>());
}

template<typename T>
bool EpsilonEqualsZero(T input)
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

inline Box TransformedAABB(Box aabb, const glm::mat4& mat)
{
	const auto center = (aabb.min + aabb.max) / 2.f;
	const auto extend = (aabb.max - aabb.min) / 2.f;

	const auto newCenter = glm::vec3(mat * glm::vec4(center, 1.f));

	glm::mat4 absMat{};

	for (auto j = 0; j < 3; ++j) {
		for (auto i = 0; i < 3; ++i) {
			absMat[i][j] = glm::abs(mat[i][j]);
		}
	}

	const auto newExtend = glm::vec3(absMat * glm::vec4(extend, 0.f));

	return { newCenter - newExtend, newCenter + newExtend };
}

// TODO: add Intersection.h to core
inline void NormalizePlane(Plane& plane)
{
	const auto mag = glm::length(glm::vec3(plane.a, plane.b, plane.c));
	plane.a = plane.a / mag;
	plane.b = plane.b / mag;
	plane.c = plane.c / mag;
	plane.d = plane.d / mag;
}

// signed distance in units of magnitude of the plane's vector n = (a,b,c)
// to obtain true distance, normalize the plane
inline float PlaneToPointDistance(const Plane& plane, glm::vec3 pt)
{
	return plane.a * pt.x + plane.b * pt.y + plane.c * pt.z + plane.d;
}

// Hartmann/Gribbs method: see paper for usage
template<bool normalize = true>
void ExtractFrustumPlanes(Frustum& f, const glm::mat4& comboMatrix)
{
	// Left clipping plane
	f.planes[Frustum::LEFT].a = comboMatrix[0][3] + comboMatrix[0][0];
	f.planes[Frustum::LEFT].b = comboMatrix[1][3] + comboMatrix[1][0];
	f.planes[Frustum::LEFT].c = comboMatrix[2][3] + comboMatrix[2][0];
	f.planes[Frustum::LEFT].d = comboMatrix[3][3] + comboMatrix[3][0];

	// Right clipping plane
	f.planes[Frustum::RIGHT].a = comboMatrix[0][3] - comboMatrix[0][0];
	f.planes[Frustum::RIGHT].b = comboMatrix[1][3] - comboMatrix[1][0];
	f.planes[Frustum::RIGHT].c = comboMatrix[2][3] - comboMatrix[2][0];
	f.planes[Frustum::RIGHT].d = comboMatrix[3][3] - comboMatrix[3][0];

	// Top clipping plane
	f.planes[Frustum::TOP].a = comboMatrix[0][3] - comboMatrix[0][1];
	f.planes[Frustum::TOP].b = comboMatrix[1][3] - comboMatrix[1][1];
	f.planes[Frustum::TOP].c = comboMatrix[2][3] - comboMatrix[2][1];
	f.planes[Frustum::TOP].d = comboMatrix[3][3] - comboMatrix[3][1];

	// Bottom clipping plane
	f.planes[Frustum::BOTTOM].a = comboMatrix[0][3] + comboMatrix[0][1];
	f.planes[Frustum::BOTTOM].b = comboMatrix[1][3] + comboMatrix[1][1];
	f.planes[Frustum::BOTTOM].c = comboMatrix[2][3] + comboMatrix[2][1];
	f.planes[Frustum::BOTTOM].d = comboMatrix[3][3] + comboMatrix[3][1];

	// Near clipping plane
	f.planes[Frustum::NEAR_].a = comboMatrix[0][3] + comboMatrix[0][2];
	f.planes[Frustum::NEAR_].b = comboMatrix[1][3] + comboMatrix[1][2];
	f.planes[Frustum::NEAR_].c = comboMatrix[2][3] + comboMatrix[2][2];
	f.planes[Frustum::NEAR_].d = comboMatrix[3][3] + comboMatrix[3][2];

	// Far clipping plane
	f.planes[Frustum::FAR_].a = comboMatrix[0][3] - comboMatrix[0][2];
	f.planes[Frustum::FAR_].b = comboMatrix[1][3] - comboMatrix[1][2];
	f.planes[Frustum::FAR_].c = comboMatrix[2][3] - comboMatrix[2][2];
	f.planes[Frustum::FAR_].d = comboMatrix[3][3] - comboMatrix[3][2];

	// Normalize the plane equations, if requested
	if constexpr (normalize) {
		NormalizePlane(f.planes[0]);
		NormalizePlane(f.planes[1]);
		NormalizePlane(f.planes[2]);
		NormalizePlane(f.planes[3]);
		NormalizePlane(f.planes[4]);
		NormalizePlane(f.planes[5]);
	}
}

enum class Intersection
{
	OUTSIDE,
	INSIDE,
	INTERSECT
};

// considers b and f are in same space
inline Intersection BoxFrustumIntersectionTest(Box b, const Frustum& f)
{
	auto res = Intersection::INSIDE;

	// for each plane do ...
	for (auto& pl : f.planes) {

		const auto planeNormal = glm::vec3(pl.a, pl.b, pl.c);

		auto vertexP = b.min;
		if (planeNormal.x >= 0)
			vertexP.x = b.max.x;
		if (planeNormal.y >= 0)
			vertexP.y = b.max.y;
		if (planeNormal.z >= 0)
			vertexP.z = b.max.z;

		auto vertexN = b.max;
		if (planeNormal.x >= 0)
			vertexN.x = b.min.x;
		if (planeNormal.y >= 0)
			vertexN.y = b.min.y;
		if (planeNormal.z >= 0)
			vertexN.z = b.min.z;

		if (PlaneToPointDistance(pl, vertexP) < 0) {
			return Intersection::OUTSIDE;
		}

		if (PlaneToPointDistance(pl, vertexN) < 0) {
			res = Intersection::INTERSECT;
		}
	}
	return res;
}

// WIP: name?
inline bool BoxFrustumCollision(Box b, const Frustum& f)
{
	switch (BoxFrustumIntersectionTest(b, f)) {
		case Intersection::INSIDE:
		case Intersection::INTERSECT: return true;
		case Intersection::OUTSIDE:;
	}
	return false;
}

inline glm::vec3 IntersectionOfThreePlanes(Plane p0, Plane p1, Plane p2)
{
	const auto a0 = p0.a;
	const auto b0 = p0.b;
	const auto c0 = p0.c;
	const auto d0 = p0.d;

	const auto a1 = p1.a;
	const auto b1 = p1.b;
	const auto c1 = p1.c;
	const auto d1 = p1.d;

	const auto a2 = p2.a;
	const auto b2 = p2.b;
	const auto c2 = p2.c;
	const auto d2 = p2.d;

	glm::vec3 intr;
	intr.x = -(-b0 * c1 * d2 + b0 * c2 * d1 + b1 * c0 * d2 - b1 * c2 * d0 - b2 * c0 * d1 + b2 * c1 * d0)
			 / (-a0 * b1 * c2 + a0 * b2 * c1 + a1 * b0 * c2 - a1 * b2 * c0 - a2 * b0 * c1 + a2 * b1 * c0);
	intr.y = -(a0 * c1 * d2 - a0 * c2 * d1 - a1 * c0 * d2 + a1 * c2 * d0 + a2 * c0 * d1 - a2 * c1 * d0)
			 / (-a0 * b1 * c2 + a0 * b2 * c1 + a1 * b0 * c2 - a1 * b2 * c0 - a2 * b0 * c1 + a2 * b1 * c0);
	intr.z = -(-a0 * b1 * d2 + a0 * b2 * d1 + a1 * b0 * d2 - a1 * b2 * d0 - a2 * b0 * d1 + a2 * b1 * d0)
			 / (-a0 * b1 * c2 + a0 * b2 * c1 + a1 * b0 * c2 - a1 * b2 * c0 - a2 * b0 * c1 + a2 * b1 * c0);

	return intr;
}

// Atten: PYRAMID
inline Box CreateBoxFromFrustumPyramid(glm::vec3 center, const Frustum& f)
{
	Box box{};

	auto ftr
		= math::IntersectionOfThreePlanes(f.planes[Frustum::TOP], f.planes[Frustum::RIGHT], f.planes[Frustum::FAR_]);

	box.min = ftr;
	box.max = ftr;

	auto ftl
		= math::IntersectionOfThreePlanes(f.planes[Frustum::TOP], f.planes[Frustum::LEFT], f.planes[Frustum::FAR_]);

	box.min = glm::min(box.min, ftl);
	box.max = glm::max(box.max, ftl);

	auto fbr
		= math::IntersectionOfThreePlanes(f.planes[Frustum::BOTTOM], f.planes[Frustum::RIGHT], f.planes[Frustum::FAR_]);

	box.min = glm::min(box.min, fbr);
	box.max = glm::max(box.max, fbr);

	auto fbl
		= math::IntersectionOfThreePlanes(f.planes[Frustum::BOTTOM], f.planes[Frustum::LEFT], f.planes[Frustum::FAR_]);

	box.min = glm::min(box.min, fbl);
	box.max = glm::max(box.max, fbl);

	box.min = glm::min(box.min, center);
	box.max = glm::max(box.max, center);

	return box;
}

} // namespace math
