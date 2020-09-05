#include "pch.h"
#include "PointlightComponent.h"

#include "rendering/scene/ScenePointlight.h"

DECLARE_DIRTY_FUNC(CPointlight)(BasicComponent& bc)
{
	return [=](ScenePointlight& pl) {
		pl.ubo.position = glm::vec4(bc.world().position, 1.f);

		if constexpr (FullDirty) {
			pl.ubo.color = glm::vec4(color, 1.f);
			pl.ubo.intensity = intensity;
			pl.ubo.constantTerm = constantTerm;
			pl.ubo.linearTerm = linearTerm;
			pl.ubo.quadraticTerm = quadraticTerm;
		}
	};
}
