#pragma once

#include <glm/glm.hpp>

namespace math {

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
	// TEST:
	[[nodiscard]] float DistanceFromPoint(glm::vec3 pt) const { return a * pt.x + b * pt.y + c * pt.z + d; }
};

// TEST:
inline glm::vec3 threePlaneIntersection(Plane p0, Plane p1, Plane p2)
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

} // namespace math
