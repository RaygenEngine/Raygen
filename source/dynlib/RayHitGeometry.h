#pragma once

#include "assets/shared/GeometryShared.h"

extern "C" {
// Size is the raw count of indicies (3 * triangleCount)
// AnyHit will stop at first hit found (faster than chit)
bool RayAnyHitGeometry(glm::vec3 start, glm::vec3 dir, const uint32* ind, const Vertex* vtx, size_t size);

// Closest hit will test everything and find the closest hit (geometry is double sided)
// Returns distance from start to the hit point: (hitPoint = start + dir * RayClosestHitGeom)
// Returns float_max on miss
// Size is the raw count of indicies (3 * triangleCount)
float RayClosestHitGeometry(glm::vec3 start, glm::vec3 dir, const uint32* ind, const Vertex* vtx, size_t size);
}
