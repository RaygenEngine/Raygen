#pragma once
#include "core/math-ext/BVH.h"

struct PhysicsIntersection {
	std::unique_ptr<math::BVH<Entity>> tree;

	struct RayCastResult {
		std::map<float, Entity> entitiesHit;
	};


	[[nodiscard]] RayCastResult RayCastDirection(glm::vec3 start, glm::vec3 direction, float distance = 99000.f) const;
	[[nodiscard]] RayCastResult RayCast(glm::vec3 start, glm::vec3 end) const;

	void Create(World& world);
};
