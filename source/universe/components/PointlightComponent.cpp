#include "PointlightComponent.h"

#include "rendering/scene/ScenePointlight.h"

DECLARE_DIRTY_FUNC(CPointlight)(BasicComponent& bc)
{
	// WIP: PERF
	auto lightP = color * intensity;
	float lightMax = std::fmaxf(std::fmaxf(lightP.x, lightP.y), lightP.z);
	float radius
		= (-linearTerm
			  + std::sqrtf(linearTerm * linearTerm - 4.f * quadraticTerm * (constantTerm - (256.f / 5.f) * lightMax)))
		  / (2.f * quadraticTerm);

	volumeTransform = math::transformMat({ radius, radius, radius }, {}, bc.world().position);

	return [=, position = bc.world().position](ScenePointlight& pl) {
		pl.ubo.position = glm::vec4(position, 1.f);
		pl.volumeTransform = volumeTransform;

		if constexpr (FullDirty) {
			pl.ubo.color = glm::vec4(color, 1.f);
			pl.ubo.intensity = intensity;
			pl.ubo.constantTerm = constantTerm;
			pl.ubo.linearTerm = linearTerm;
			pl.ubo.quadraticTerm = quadraticTerm;
			pl.ubo.hasShadow = hasShadow;
			pl.ubo.samples = samples;
		}
	};
}
