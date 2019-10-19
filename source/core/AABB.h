#pragma once

#include "CoreStructs.h"

namespace aabb {


inline bool Overlap(const Box& bb0, const Box& bb1)
{
	if (bb0.min.x > bb1.max.x || bb0.max.x < bb1.min.x)
		return false;

	if (bb0.min.y > bb1.max.y || bb0.max.y < bb1.min.y)
		return false;

	if (bb0.min.z > bb1.max.z || bb0.max.z < bb1.min.z)
		return false;

	return true;
}
}; // namespace aabb
