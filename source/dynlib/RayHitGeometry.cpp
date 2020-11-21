#include "RayHitGeometry.h"

static bool RayTriangleIntersect(
	glm::vec3 rayOrig, glm::vec3 rayDir, glm::vec3 vertex0, glm::vec3 vertex1, glm::vec3 vertex2, float& t)
{
	t = -1.f;
	glm::vec3 edge1 = vertex1 - vertex0;
	glm::vec3 edge2 = vertex2 - vertex0;

	glm::vec3 h = glm::cross(rayDir, edge2);
	float a = glm::dot(edge1, h);

	if (math::equalsZero(a)) {
		return false;
	}

	float f = 1.0 / a;
	glm::vec3 s = rayOrig - vertex0;

	float u = f * (glm::dot(s, h));

	// PERF: Optimize branch prediction here
	if (u < 0.0 || u > 1.0) {
		return false;
	}

	glm::vec3 q = glm::cross(s, edge1);
	float v = f * glm::dot(rayDir, q);

	if (v < 0.0 || u + v > 1.0) {
		return false;
	}

	t = f * glm::dot(edge2, q);
	return t > 0;
}


extern "C" {

bool RayAnyHitGeometry(glm::vec3 start, glm::vec3 dir, const uint32* ind, const Vertex* vtx, size_t size)
{
	for (int32 i = 0; i < size; i += 3) {
		float outT = 0.f;
		if (RayTriangleIntersect(         //
				start,                    //
				dir,                      //
				vtx[ind[i]].position,     //
				vtx[ind[i + 1]].position, //
				vtx[ind[i + 2]].position, //
				outT)) {

			return true;
		}
	}
	return false;
}


float RayClosestHitGeometry(glm::vec3 start, glm::vec3 dir, const uint32* ind, const Vertex* vtx, size_t size)
{
	constexpr float fmax = std::numeric_limits<float>::max();
	float closestHit = fmax;
	for (int32 i = 0; i < size; i += 3) {
		float outT = fmax;
		if (RayTriangleIntersect(         //
				start,                    //
				dir,                      //
				vtx[ind[i]].position,     //
				vtx[ind[i + 1]].position, //
				vtx[ind[i + 2]].position, //
				outT)) {

			if (outT < closestHit) {
				closestHit = outT;
			}
		}
	}

	return closestHit;
}

} /// extern C
