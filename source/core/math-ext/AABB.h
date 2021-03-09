#pragma once
#include <glm/glm.hpp>

namespace math {

namespace detail {
	// MATH: refactor this copy pasta
	inline bool GetIntersection(float fDst1, float fDst2, glm::vec3 P1, glm::vec3 P2, glm::vec3& Hit)
	{
		if ((fDst1 * fDst2) >= 0.0f) {
			return false;
		}

		Hit = P1 + (P2 - P1) * (-fDst1 / (fDst2 - fDst1));
		return true;
	}

	inline bool InBox(glm::vec3 Hit, glm::vec3 B1, glm::vec3 B2, const int Axis)
	{
		if (Axis == 1 && Hit.z >= B1.z && Hit.z <= B2.z && Hit.y >= B1.y && Hit.y <= B2.y)
			return true;
		if (Axis == 2 && Hit.z >= B1.z && Hit.z <= B2.z && Hit.x >= B1.x && Hit.x <= B2.x)
			return true;
		if (Axis == 3 && Hit.x >= B1.x && Hit.x <= B2.x && Hit.y >= B1.y && Hit.y <= B2.y)
			return true;
		return false;
	}

	// returns true if line (L1, L2) intersects with the box (B1, B2)
	// returns intersection point in Hit
	inline bool CheckLineBox(glm::vec3 B1, glm::vec3 B2, glm::vec3 L1, glm::vec3 L2, glm::vec3& Hit)
	{
		if (L2.x <= B1.x && L1.x <= B1.x)
			return false;
		if (L2.x >= B2.x && L1.x >= B2.x)
			return false;
		if (L2.y <= B1.y && L1.y <= B1.y)
			return false;
		if (L2.y >= B2.y && L1.y >= B2.y)
			return false;
		if (L2.z <= B1.z && L1.z <= B1.z)
			return false;
		if (L2.z >= B2.z && L1.z >= B2.z)
			return false;
		if (L1.x >= B1.x && L1.x <= B2.x && L1.y >= B1.y && L1.y <= B2.y && L1.z >= B1.z && L1.z <= B2.z) {
			Hit = L1;
			return true;
		}
		if ((GetIntersection(L1.x - B1.x, L2.x - B1.x, L1, L2, Hit) && InBox(Hit, B1, B2, 1))
			|| (GetIntersection(L1.y - B1.y, L2.y - B1.y, L1, L2, Hit) && InBox(Hit, B1, B2, 2))
			|| (GetIntersection(L1.z - B1.z, L2.z - B1.z, L1, L2, Hit) && InBox(Hit, B1, B2, 3))
			|| (GetIntersection(L1.x - B2.x, L2.x - B2.x, L1, L2, Hit) && InBox(Hit, B1, B2, 1))
			|| (GetIntersection(L1.y - B2.y, L2.y - B2.y, L1, L2, Hit) && InBox(Hit, B1, B2, 2))
			|| (GetIntersection(L1.z - B2.z, L2.z - B2.z, L1, L2, Hit) && InBox(Hit, B1, B2, 3)))
			return true;

		return false;
	}

	// Return exact hitpoint
	inline bool CheckLineBoxExact(glm::vec3 B1, glm::vec3 B2, glm::vec3 L1, glm::vec3 L2, glm::vec3& Hit)
	{
		if (L2.x <= B1.x && L1.x <= B1.x)
			return false;
		if (L2.x >= B2.x && L1.x >= B2.x)
			return false;
		if (L2.y <= B1.y && L1.y <= B1.y)
			return false;
		if (L2.y >= B2.y && L1.y >= B2.y)
			return false;
		if (L2.z <= B1.z && L1.z <= B1.z)
			return false;
		if (L2.z >= B2.z && L1.z >= B2.z)
			return false;
		if (L1.x >= B1.x && L1.x <= B2.x && L1.y >= B1.y && L1.y <= B2.y && L1.z >= B1.z && L1.z <= B2.z) {
			Hit = L1;
			return true;
		}

		glm::vec3 tempHit;
		float dstSq = std::numeric_limits<float>::max();

		// PERF: super dumb way to find correct hitpoint
		if (GetIntersection(L1.x - B1.x, L2.x - B1.x, L1, L2, tempHit) && InBox(tempHit, B1, B2, 1)
			&& glm::distance2(L1, tempHit) < dstSq) {
			Hit = tempHit;
			dstSq = glm::distance2(L1, tempHit);
		}
		if (GetIntersection(L1.y - B1.y, L2.y - B1.y, L1, L2, tempHit) && InBox(tempHit, B1, B2, 2)
			&& glm::distance2(L1, tempHit) < dstSq) {
			Hit = tempHit;
			dstSq = glm::distance2(L1, tempHit);
		}
		if (GetIntersection(L1.z - B1.z, L2.z - B1.z, L1, L2, tempHit) && InBox(tempHit, B1, B2, 3)
			&& glm::distance2(L1, tempHit) < dstSq) {
			Hit = tempHit;
			dstSq = glm::distance2(L1, tempHit);
		}
		if (GetIntersection(L1.x - B2.x, L2.x - B2.x, L1, L2, tempHit) && InBox(tempHit, B1, B2, 1)
			&& glm::distance2(L1, tempHit) < dstSq) {
			Hit = tempHit;
			dstSq = glm::distance2(L1, tempHit);
		}
		if (GetIntersection(L1.y - B2.y, L2.y - B2.y, L1, L2, tempHit) && InBox(tempHit, B1, B2, 2)
			&& glm::distance2(L1, tempHit) < dstSq) {
			Hit = tempHit;
			dstSq = glm::distance2(L1, tempHit);
		}
		if (GetIntersection(L1.z - B2.z, L2.z - B2.z, L1, L2, tempHit) && InBox(tempHit, B1, B2, 3)
			&& glm::distance2(L1, tempHit) < dstSq) {
			Hit = tempHit;
			dstSq = glm::distance2(L1, tempHit);
		}

		return dstSq < std::numeric_limits<float>::max();
	}
} // namespace detail


struct AABB {
	glm::vec3 min{};
	glm::vec3 max{};

	AABB() = default;

	AABB(glm::vec3 min_, glm::vec3 max_)
		: min(min_)
		, max(max_)
	{
	}

	[[nodiscard]] glm::vec3 GetCenter() const { return (min + max) / 2.f; }
	[[nodiscard]] glm::vec3 GetExtend() const { return (max - min) / 2.f; }

	// TEST: (check for tighter fits)
	[[nodiscard]] AABB Transform(const glm::mat4& mat) const
	{
		const auto newCenter = glm::vec3(mat * glm::vec4(GetCenter(), 1.f));

		glm::mat4 absMat{};

		for (auto j = 0; j < 3; ++j) {
			for (auto i = 0; i < 3; ++i) {
				absMat[i][j] = glm::abs(mat[i][j]);
			}
		}

		const auto newExtend = glm::vec3(absMat * glm::vec4(GetExtend(), 0.f));

		AABB newAabb;
		newAabb.min = newCenter - newExtend;
		newAabb.max = newCenter + newExtend;
		return newAabb;
	}

	// TEST:
	[[nodiscard]] constexpr bool Intersects(const AABB& other) const
	{
		if (other.min.x > max.x || other.max.x < min.x) {
			return false;
		}
		if (other.min.y > max.y || other.max.y < min.y) {
			return false;
		}
		if (other.min.z > max.z || other.max.z < min.z) {
			return false;
		}

		return true;
	}


	[[nodiscard]] constexpr bool IsInside(glm::vec3 point) const
	{
		// PERF: if used a lot, benchmark branch predictor
		// Check if the point is less than max and greater than min
		if (point.x >= min.x
			&& point.x <= max.x
			//
			&& point.y >= min.y
			&& point.y <= max.y
			//
			&& point.z >= min.z && point.z <= max.z
			//
		) {
			return true;
		}

		return false;
	}

	[[nodiscard]] glm::vec3 ClampInside(glm::vec3 point, glm::vec3 margin = glm::vec3{ 0.f }) const
	{
		return glm::clamp(point, min + margin, max - margin);
	}

	[[nodiscard]] AABB Union(AABB rhs) const
	{
		//
		return AABB(glm::min(min, rhs.min), glm::max(max, rhs.max));
	}

	[[nodiscard]] float GetArea() const
	{
		glm::vec3 v = max - min;
		return 2.f * (v.x * v.y + v.y * v.z + v.z * v.x);
	}

	[[nodiscard]] bool Overlaps(glm::vec3 a, glm::vec3 b) const
	{
		glm::vec3 hit{};
		return detail::CheckLineBox(min, max, a, b, hit);
	}

	[[nodiscard]] bool OverlapsHitPoint(glm::vec3 a, glm::vec3 b, glm::vec3& hitPoint) const
	{
		// MATH: what to return in this hitpoint
		if (IsInside(a)) {
			hitPoint = a;
			return true;
		}
		if (IsInside(b)) {
			hitPoint = b;
			return true;
		}

		return detail::CheckLineBox(min, max, a, b, hitPoint);
	}

	[[nodiscard]] bool OverlapsExactHitPoint(glm::vec3 a, glm::vec3 b, glm::vec3& hitPoint) const
	{
		if (IsInside(a)) {
			hitPoint = a;
			return true;
		}
		if (IsInside(b)) {
			hitPoint = b;
			return true;
		}

		return detail::CheckLineBoxExact(min, max, a, b, hitPoint);
	}
};

} // namespace math
