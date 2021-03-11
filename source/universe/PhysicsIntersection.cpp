#include "PhysicsIntersection.h"

#include "assets/pods/Mesh.h"
#include "engine/Timer.h"
#include "universe/World.h"
#include "universe/components/StaticMeshComponent.h"

using RayCastResult = PhysicsIntersection::RayCastResult;
using RayCastEntityHit = PhysicsIntersection::RayCastEntityHit;

namespace {
std::pair<glm::vec3, glm::vec3> RayToEntityMeshSpace(Entity ent, glm::vec3 start, glm::vec3 end)
{
	// const Mesh& mesh = *ent.Get<CStaticMesh>().mesh.Lock(); NEW::
	// glm::mat4 invTransform = glm::inverse(ent->world().transform);

	// glm::vec4 trStart = invTransform * glm::vec4(start, 1.f);
	// glm::vec4 trEnd = invTransform * glm::vec4(end, 1.f);
	// return { glm::vec3(trStart), glm::normalize(glm::vec3(trEnd - trStart)) };
	return {};
}
} // namespace

PhysicsIntersection::PhysicsIntersection()
{
	AnyHitFunc = loader.LibLoadFunc(LIBLOAD_FUNC(RayAnyHitGeometry));
	ClosestHitFunc = loader.LibLoadFunc(LIBLOAD_FUNC(RayClosestHitGeometry));
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

		auto [rayOrigin, rayDir] = RayToEntityMeshSpace(ent, start, end - start);

		for (const GeometrySlot& geom : mesh.geometrySlots) {
			// NOTE: using raw ptrs for debug performance here. (no index debugging)
			const uint32* ind = geom.indices.data();
			const Vertex* vtx = geom.vertices.data();

			const auto size = geom.indices.size();
			LOG_REPORT("Vtx: {}", size / 3);

			if (AnyHitFunc(rayOrigin, rayDir, ind, vtx, size)) {
				return false;
			};
		}
		// Did not hit anything in static mesh
		return true;
	});


	return { std::move(results.distanceSqToHitObject) };
}

RayCastEntityHit PhysicsIntersection::RayCastChitSelection(glm::vec3 start, glm::vec3 end) const
{
	RayCastEntityHit result;
	// TIMER_SCOPE("RayCast");
	auto originalHits = tree->RayCastExactHit(start, end);

	result.distance = std::numeric_limits<float>::max();

	for (auto [distanceSq, ent] : originalHits.distanceSqToHitObject) {
		if (distanceSq > 0 && distanceSq > result.distance * result.distance) {
			return result;
		}

		if (!ent.Has<CStaticMesh>()) {
			if (distanceSq < result.distance * result.distance) {
				result.entity = ent;
				result.geomGroupIndex = -1;
				result.distance = glm::sqrt(distanceSq);
			}
			continue;
		}


		auto [rayOrigin, rayDir] = RayToEntityMeshSpace(ent, start, end);

		const Mesh& mesh = *ent.Get<CStaticMesh>().mesh.Lock();
		for (int32 i = 0; const GeometrySlot& geom : mesh.geometrySlots) {
			auto thisDist
				= ClosestHitFunc(rayOrigin, rayDir, geom.indices.data(), geom.vertices.data(), geom.indices.size());

			if (thisDist < result.distance) {
				result.entity = ent;
				result.geomGroupIndex = i;
				result.distance = thisDist;
			}
			++i;
		}
	}


	return result;
}

// MATH: remove copy paste
RayCastEntityHit PhysicsIntersection::RayCastChitGeom(glm::vec3 start, glm::vec3 end) const
{
	RayCastEntityHit result;
	TIMER_SCOPE("RayCast");
	auto originalHits = tree->RayCastExactHit(start, end);

	result.distance = std::numeric_limits<float>::max();

	for (auto [distanceSq, ent] : originalHits.distanceSqToHitObject) {
		if (distanceSq > 0 && distanceSq > result.distance * result.distance) {
			return result;
		}

		if (!ent.Has<CStaticMesh>()) {
			continue;
		}

		auto [rayOrigin, rayDir] = RayToEntityMeshSpace(ent, start, end);

		const Mesh& mesh = *ent.Get<CStaticMesh>().mesh.Lock();
		for (int32 i = 0; const GeometrySlot& geom : mesh.geometrySlots) {
			auto thisDist
				= ClosestHitFunc(rayOrigin, rayDir, geom.indices.data(), geom.vertices.data(), geom.indices.size());

			if (thisDist < result.distance) {
				result.entity = ent;
				result.geomGroupIndex = i;
				result.distance = thisDist;
			}
			++i;
		}
	}

	return result;
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

		// box = box.Transform(bc.world().transform()); NEW::
		tree->InsertLeaf(bc.self, box);
	}
}
