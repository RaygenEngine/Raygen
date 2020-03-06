#pragma once

#include <glm/glm.hpp>

namespace math {

struct AABB {
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
	[[nodiscard]] bool Intersects(const AABB& other) const
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
};

} // namespace math
