#pragma once
#include <glm/glm.hpp>

namespace math {

constexpr struct AABB {
	glm::vec3 min{};
	glm::vec3 max{};

	[[nodiscard]] glm::vec3 GetCenter() const { return (min + max) / 2.f; }
	[[nodiscard]] glm::vec3 GetExtend() const { return (max - min) / 2.f; }

	// TEST: (check for tighter fits)
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

	// TEST:
	[[nodiscard]] constexpr bool Intersects(const AABB& other)
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
};

} // namespace math
