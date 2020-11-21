#pragma once
#include "core/math-ext/BVH.h"
#include "platform/DynLibLoader.h"
#include "dynlib/RayHitGeometry.h"

struct PhysicsIntersection {
	std::unique_ptr<math::BVH<Entity>> tree;
	DynLibLoader loader{ "Raygen-Dynlib" };

	decltype(&RayAnyHitGeometry) AnyHitFunc;
	decltype(&RayClosestHitGeometry) ClosestHitFunc;


	PhysicsIntersection();

	struct RayCastResult {
		std::map<float, Entity> entitiesHit;
	};

	struct RayCastEntityHit {
		Entity entity;
		// Can be -1 if the ray hit an entity without geometry
		int32 geomGroupIndex{ -1 };
		float distance{ std::numeric_limits<float>::max() };
	};


	[[nodiscard]] RayCastResult RayCastDirection(glm::vec3 start, glm::vec3 direction, float distance = 99000.f) const;
	[[nodiscard]] RayCastResult RayCast(glm::vec3 start, glm::vec3 end) const;

	[[nodiscard]] RayCastEntityHit RayCastChitSelection(glm::vec3 start, glm::vec3 end) const;
	[[nodiscard]] RayCastEntityHit RayCastChitGeom(glm::vec3 start, glm::vec3 end) const;


	void Create(World& world);
};
