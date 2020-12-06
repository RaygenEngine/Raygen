#include "QuadlightComponent.h"

#include "rendering/scene/SceneQuadlight.h"

DECLARE_DIRTY_FUNC(CQuadlight)(BasicComponent& bc)
{
	return [=, center = bc.world().position, front = bc.world().front(), up = bc.world().up(),
			   right = bc.world().right(), scale = bc.world().scale](SceneQuadlight& ql) {
		ql.ubo.center = glm::vec4(center, 1.f);
		ql.ubo.right = glm::vec4(right, width);
		ql.ubo.up = glm::vec4(up, height);
		ql.ubo.normal = glm::vec4(front, 0.f);

		if constexpr (FullDirty) {
			ql.ubo.color = glm::vec4(color, 1.f);
			ql.ubo.intensity = intensity;
			ql.ubo.constantTerm = constantTerm;
			ql.ubo.linearTerm = linearTerm;
			ql.ubo.quadraticTerm = quadraticTerm;
			ql.ubo.hasShadow = hasShadow;
			ql.ubo.samples = samples;
			ql.ubo.radius = radius;
		}
	};
}
