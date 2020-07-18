#pragma once
#include "core/math-ext/AABB.h"
#include "core/math-ext/Plane.h"

#include <array>


namespace math {

enum class Intersection
{
	Outside,
	Inside,
	Intersect
};

struct Frustum {
	enum : size_t
	{
		_top_ = 0,
		_bottom_,
		_left_,
		_right_,
		_near_,
		_far_
	};

	std::array<Plane, 6> planes;

	// TEST:
	// Hartmann/Gribbs method: see paper for usage
	template<bool normalize = true>
	void ExtractFromMatrix(const glm::mat4& comboMatrix)
	{
		// Left clipping plane
		planes[_left_].a = comboMatrix[0][3] + comboMatrix[0][0];
		planes[_left_].b = comboMatrix[1][3] + comboMatrix[1][0];
		planes[_left_].c = comboMatrix[2][3] + comboMatrix[2][0];
		planes[_left_].d = comboMatrix[3][3] + comboMatrix[3][0];

		// Right clipping plane
		planes[_right_].a = comboMatrix[0][3] - comboMatrix[0][0];
		planes[_right_].b = comboMatrix[1][3] - comboMatrix[1][0];
		planes[_right_].c = comboMatrix[2][3] - comboMatrix[2][0];
		planes[_right_].d = comboMatrix[3][3] - comboMatrix[3][0];

		// Top clipping plane
		planes[_top_].a = comboMatrix[0][3] - comboMatrix[0][1];
		planes[_top_].b = comboMatrix[1][3] - comboMatrix[1][1];
		planes[_top_].c = comboMatrix[2][3] - comboMatrix[2][1];
		planes[_top_].d = comboMatrix[3][3] - comboMatrix[3][1];

		// Bottom clipping plane
		planes[_bottom_].a = comboMatrix[0][3] + comboMatrix[0][1];
		planes[_bottom_].b = comboMatrix[1][3] + comboMatrix[1][1];
		planes[_bottom_].c = comboMatrix[2][3] + comboMatrix[2][1];
		planes[_bottom_].d = comboMatrix[3][3] + comboMatrix[3][1];

		// Near clipping plane
		planes[_near_].a = comboMatrix[0][3] + comboMatrix[0][2];
		planes[_near_].b = comboMatrix[1][3] + comboMatrix[1][2];
		planes[_near_].c = comboMatrix[2][3] + comboMatrix[2][2];
		planes[_near_].d = comboMatrix[3][3] + comboMatrix[3][2];

		// Far clipping plane
		planes[_far_].a = comboMatrix[0][3] - comboMatrix[0][2];
		planes[_far_].b = comboMatrix[1][3] - comboMatrix[1][2];
		planes[_far_].c = comboMatrix[2][3] - comboMatrix[2][2];
		planes[_far_].d = comboMatrix[3][3] - comboMatrix[3][2];

		// Normalize the plane equations, if requested
		if constexpr (normalize) {
			for (auto& p : planes) {
				p.Normalize();
			}
		}
	}

	// TEST:
	// considers aabb and this frustum are in same space
	[[nodiscard]] Intersection IntersectionTest(AABB aabb)
	{
		auto res = Intersection::Inside;

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
				return Intersection::Outside;
			}

			if (pl.DistanceFromPoint(vertexN) < 0) {
				res = Intersection::Intersect;
			}
		}
		return res;
	}

	// TEST:
	[[nodiscard]] bool Intersects(AABB aabb)
	{
		switch (IntersectionTest(aabb)) {
			case Intersection::Inside:
			case Intersection::Intersect: return true;
			case Intersection::Outside:;
		}
		return false;
	}

	// TEST:
	// CHECK: for better precision create using all points of the frustum
	[[nodiscard]] AABB FrustumPyramidAABB(glm::vec3 apex) const
	{
		AABB box{};

		auto ftr = threePlaneIntersection(planes[_top_], planes[_right_], planes[_far_]);

		box.min = ftr;
		box.max = ftr;

		auto ftl = threePlaneIntersection(planes[_top_], planes[_left_], planes[_far_]);

		box.min = glm::min(box.min, ftl);
		box.max = glm::max(box.max, ftl);

		auto fbr = threePlaneIntersection(planes[_bottom_], planes[_right_], planes[_far_]);

		box.min = glm::min(box.min, fbr);
		box.max = glm::max(box.max, fbr);

		auto fbl = threePlaneIntersection(planes[_bottom_], planes[_left_], planes[_far_]);

		box.min = glm::min(box.min, fbl);
		box.max = glm::max(box.max, fbl);

		box.min = glm::min(box.min, apex);
		box.max = glm::max(box.max, apex);

		return box;
	}

	// PERF: general perf frustum this is expensive
	[[nodiscard]] std::array<glm::vec3, 8> GetPoints()
	{

		std::array<glm::vec3, 8> points{};

		points[0] = threePlaneIntersection(planes[_top_], planes[_right_], planes[_far_]);
		points[1] = threePlaneIntersection(planes[_top_], planes[_left_], planes[_far_]);
		points[2] = threePlaneIntersection(planes[_bottom_], planes[_right_], planes[_far_]);
		points[3] = threePlaneIntersection(planes[_bottom_], planes[_left_], planes[_far_]);

		points[4] = threePlaneIntersection(planes[_bottom_], planes[_left_], planes[_near_]);
		points[5] = threePlaneIntersection(planes[_bottom_], planes[_left_], planes[_near_]);
		points[6] = threePlaneIntersection(planes[_bottom_], planes[_left_], planes[_near_]);
		points[7] = threePlaneIntersection(planes[_bottom_], planes[_left_], planes[_near_]);

		return points;
	}
};

} // namespace math
