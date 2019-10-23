#pragma once

#include <array>
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
	const auto s = glm::scale(scale);
	const auto r = glm::toMat4(orientation);
	const auto t = glm::translate(translation);

	return t * r * s;
}

inline glm::mat4 TransformMatrixFromTRS(glm::vec3 scale, glm::vec3 raxis, float rads, glm::vec3 translation)
{
	const auto s = glm::scale(scale);
	const auto r = glm::rotate(rads, raxis);
	const auto t = glm::translate(translation);

	return t * r * s;
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

enum class Intersection
{
	OUTSIDE,
	INSIDE,
	INTERSECT
};

struct AABB {
	glm::vec3 min{};
	glm::vec3 max{};

	[[nodiscard]] glm::vec3 GetCenter() const { return (min + max) / 2.f; }
	[[nodiscard]] glm::vec3 GetExtend() const { return (max - min) / 2.f; }

	void Transform(const glm::mat4& mat)
	{
		const auto newCenter = glm::vec3(mat * glm::vec4(GetCenter(), 1.f));

		glm::mat4 absMat{};

		for (auto j = 0; j < 3; ++j) {
			for (auto i = 0; i < 3; ++i) {
				absMat[i][j] = glm::abs(mat[i][j]);
			}
		}

		const auto newExtend = glm::vec3(absMat * glm::vec4(GetExtend(), 0.f));

		min = newCenter - newExtend;
		max = newCenter + newExtend;
	}
};

struct Plane {
	// ax + by + cz + d = 0
	float a, b, c, d;

	void Normalize()
	{
		const auto mag = glm::length(glm::vec3(a, b, c));
		a = a / mag;
		b = b / mag;
		c = c / mag;
		d = d / mag;
	}

	// signed distance in units of magnitude of the plane's vector n = (a,b,c)
	// to obtain true distance, normalize the plane
	[[nodiscard]] float DistanceFromPoint(glm::vec3 pt) const { return a * pt.x + b * pt.y + c * pt.z + d; }
};

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

struct Frustum {
	enum
	{
		TOP = 0,
		BOTTOM,
		LEFT,
		RIGHT,
		NEAR_,
		FAR_
	};

	std::array<Plane, 6> planes;

	// Hartmann/Gribbs method: see paper for usage
	template<bool normalize = true>
	void ExtractFromMatrix(const glm::mat4& comboMatrix)
	{
		// Left clipping plane
		planes[LEFT].a = comboMatrix[0][3] + comboMatrix[0][0];
		planes[LEFT].b = comboMatrix[1][3] + comboMatrix[1][0];
		planes[LEFT].c = comboMatrix[2][3] + comboMatrix[2][0];
		planes[LEFT].d = comboMatrix[3][3] + comboMatrix[3][0];

		// Right clipping plane
		planes[RIGHT].a = comboMatrix[0][3] - comboMatrix[0][0];
		planes[RIGHT].b = comboMatrix[1][3] - comboMatrix[1][0];
		planes[RIGHT].c = comboMatrix[2][3] - comboMatrix[2][0];
		planes[RIGHT].d = comboMatrix[3][3] - comboMatrix[3][0];

		// Top clipping plane
		planes[TOP].a = comboMatrix[0][3] - comboMatrix[0][1];
		planes[TOP].b = comboMatrix[1][3] - comboMatrix[1][1];
		planes[TOP].c = comboMatrix[2][3] - comboMatrix[2][1];
		planes[TOP].d = comboMatrix[3][3] - comboMatrix[3][1];

		// Bottom clipping plane
		planes[BOTTOM].a = comboMatrix[0][3] + comboMatrix[0][1];
		planes[BOTTOM].b = comboMatrix[1][3] + comboMatrix[1][1];
		planes[BOTTOM].c = comboMatrix[2][3] + comboMatrix[2][1];
		planes[BOTTOM].d = comboMatrix[3][3] + comboMatrix[3][1];

		// Near clipping plane
		planes[NEAR_].a = comboMatrix[0][3] + comboMatrix[0][2];
		planes[NEAR_].b = comboMatrix[1][3] + comboMatrix[1][2];
		planes[NEAR_].c = comboMatrix[2][3] + comboMatrix[2][2];
		planes[NEAR_].d = comboMatrix[3][3] + comboMatrix[3][2];

		// Far clipping plane
		planes[FAR_].a = comboMatrix[0][3] - comboMatrix[0][2];
		planes[FAR_].b = comboMatrix[1][3] - comboMatrix[1][2];
		planes[FAR_].c = comboMatrix[2][3] - comboMatrix[2][2];
		planes[FAR_].d = comboMatrix[3][3] - comboMatrix[3][2];

		// Normalize the plane equations, if requested
		if constexpr (normalize) {
			for (auto& p : planes) {
				p.Normalize();
			}
		}
	}

	// considers aabb and f are in same space
	[[nodiscard]] Intersection IntersectionTestWithAABB(AABB aabb)
	{
		auto res = Intersection::INSIDE;

		// for each plane do ...
		for (auto& pl : planes) {

			const auto planeNormal = glm::vec3(pl.a, pl.b, pl.c);

			auto vertexP = aabb.min;
			if (planeNormal.x >= 0)
				vertexP.x = aabb.max.x;
			if (planeNormal.y >= 0)
				vertexP.y = aabb.max.y;
			if (planeNormal.z >= 0)
				vertexP.z = aabb.max.z;

			auto vertexN = aabb.max;
			if (planeNormal.x >= 0)
				vertexN.x = aabb.min.x;
			if (planeNormal.y >= 0)
				vertexN.y = aabb.min.y;
			if (planeNormal.z >= 0)
				vertexN.z = aabb.min.z;

			if (pl.DistanceFromPoint(vertexP) < 0) {
				return Intersection::OUTSIDE;
			}

			if (pl.DistanceFromPoint(vertexN) < 0) {
				res = Intersection::INTERSECT;
			}
		}
		return res;
	}

	[[nodiscard]] bool IntersectsAABB(AABB aabb)
	{
		switch (IntersectionTestWithAABB(aabb)) {
			case Intersection::INSIDE:
			case Intersection::INTERSECT: return true;
			case Intersection::OUTSIDE:;
		}
		return false;
	}

	// TODO: for better precision create using all points of the frustum
	[[nodiscard]] AABB FrustumPyramidAABB(glm::vec3 apex)
	{
		AABB box{};

		auto ftr = IntersectionOfThreePlanes(planes[TOP], planes[RIGHT], planes[FAR_]);

		box.min = ftr;
		box.max = ftr;

		auto ftl = IntersectionOfThreePlanes(planes[TOP], planes[LEFT], planes[FAR_]);

		box.min = glm::min(box.min, ftl);
		box.max = glm::max(box.max, ftl);

		auto fbr = IntersectionOfThreePlanes(planes[BOTTOM], planes[RIGHT], planes[FAR_]);

		box.min = glm::min(box.min, fbr);
		box.max = glm::max(box.max, fbr);

		auto fbl = IntersectionOfThreePlanes(planes[BOTTOM], planes[LEFT], planes[FAR_]);

		box.min = glm::min(box.min, fbl);
		box.max = glm::max(box.max, fbl);

		box.min = glm::min(box.min, apex);
		box.max = glm::max(box.max, apex);

		return box;
	}
};

} // namespace math
