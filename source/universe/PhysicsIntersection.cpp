#include "PhysicsIntersection.h"

#include "universe/components/StaticMeshComponent.h"
#include "assets/pods/Mesh.h"
#include "engine/Timer.h"
#include "universe/World.h"

using RayCastResult = PhysicsIntersection::RayCastResult;

static bool RayTriangleIntersect(
	glm::vec3 rayOrig, glm::vec3 rayDir, glm::vec3 vertex0, glm::vec3 vertex1, glm::vec3 vertex2, float& t)
{
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
	return !math::equalsZero(t);
}

RayCastResult PhysicsIntersection::RayCastDirection(glm::vec3 start, glm::vec3 direction, float distance) const
{
	return RayCast(start, glm::normalize(direction) * distance);
}

RayCastResult PhysicsIntersection::RayCast(glm::vec3 start, glm::vec3 end) const
{
	TIMER_SCOPE("RayCast");
	auto results = tree->RayCast(start, end);

	const auto count = std::erase_if(results.distanceSqToHitObject, [&](const auto& item) {
		Entity ent = item.second;

		if (!ent.Has<CStaticMesh>()) {
			return false;
		}
		const Mesh& mesh = *ent.Get<CStaticMesh>().mesh.Lock();
		glm::mat4 invTransform = glm::inverse(ent->world().transform);

		glm::vec4 trStart = invTransform * glm::vec4(start, 1.f);
		glm::vec4 trEnd = invTransform * glm::vec4(end, 1.f);

		glm::vec3 dir = glm::vec3(trEnd - trStart);


		for (const GeometrySlot& geom : mesh.geometrySlots) {
			// NOTE: using raw ptrs for debug performance here. (no index debugging)
			const uint32* ind = geom.indices.data();
			const Vertex* vtx = geom.vertices.data();

			const auto size = geom.indices.size();
			LOG_REPORT("Vtx: {}", size / 3);
			for (int32 i = 0; i < size; i += 3) {
				float outT = 0.f;
				if (RayTriangleIntersect(         //
						trStart,                  //
						dir,                      //
						vtx[ind[i]].position,     //
						vtx[ind[i + 1]].position, //
						vtx[ind[i + 2]].position, //
						outT)) {

					// Break lambda
					return false;
				}
			}
		}
		// Did not hit anything in static mesh
		return true;
	});


	return { std::move(results.distanceSqToHitObject) };
}

void PhysicsIntersection::Create(World& world)
{
	tree = std::make_unique<math::BVH<Entity>>();

	auto view = world.GetView<BasicComponent>();
	for (auto&& [ent, bc] : view.each()) {
		glm::vec3 h = { 1, 1, 1 };
		math::AABB box = { -h, h };

		if (bc.self.Has<CStaticMesh>()) {
			auto& slots = bc.self.Get<CStaticMesh>().mesh.Lock()->geometrySlots;

			for (auto& slot : slots) {
				for (auto vtx : slot.vertices) {
					box.min = glm::min(vtx.position, box.min);
					box.max = glm::max(vtx.position, box.max);
				}
			}
		}

		box = box.Transform(bc.world().transform);
		tree->InsertLeaf(bc.self, box);
	}
}
