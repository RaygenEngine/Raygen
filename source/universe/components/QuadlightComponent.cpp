#include "QuadlightComponent.h"

#include "rendering/scene/SceneQuadlight.h"

DECLARE_DIRTY_FUNC(CQuadlight)(BasicComponent& bc)
{
	return [=, position = bc.world().position, orientation = bc.world().orientation, scale = bc.world().scale](
			   SceneQuadlight& ql) {
		ql.ubo.position = glm::vec4(position, 1.f);
		ql.ubo.front = glm::vec4(bc.world().front(), 0.f);
		ql.ubo.scaleX = scale.x;
		ql.ubo.scaleY = scale.y;

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
