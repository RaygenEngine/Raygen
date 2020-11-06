#include "PointlightComponent.h"

#include "rendering/scene/ScenePointlight.h"

DECLARE_DIRTY_FUNC(CPointlight)(BasicComponent& bc)
{
	float radius = CalculateEffectiveRadius();

	return [=, position = bc.world().position, orientation = bc.world().orientation](ScenePointlight& pl) {
		pl.ubo.position = glm::vec4(position, 1.f);
		pl.volumeTransform = math::transformMat(glm::vec3{ radius }, bc.world().orientation, bc.world().position);

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

float CPointlight::CalculateEffectiveRadius() const
{
	auto lightP = color * intensity;
	float lightMax = std::fmaxf(std::fmaxf(lightP.x, lightP.y), lightP.z);
	return (-linearTerm
			   + std::sqrtf(linearTerm * linearTerm - 4.f * quadraticTerm * (constantTerm - (256.f / 5.f) * lightMax)))
		   / (2.f * quadraticTerm);
}
