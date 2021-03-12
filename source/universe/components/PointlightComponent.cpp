#include "PointlightComponent.h"

DECLARE_DIRTY_FUNC(CPointlight)(BasicComponent& bc)
{
	float effectiveRadius = CalculateEffectiveRadius();

	return [=, translation = bc.world().translation(), orientation = bc.world().orientation()](ScenePointlight& pl) {
		XMStoreFloat3A(&pl.ubo.position, translation);
		//= math::transformMat(glm::vec3{ effectiveRadius }, bc.world().orientation, bc.world().position); NEW::

		// XMStoreFloat4x4A(&pl.volumeTransform, XMM);

		if constexpr (FullDirty) {
			pl.ubo.color = { color.x, color.y, color.z };
			pl.ubo.intensity = intensity;
			pl.ubo.constantTerm = constantTerm;
			pl.ubo.linearTerm = linearTerm;
			pl.ubo.quadraticTerm = quadraticTerm;
			pl.ubo.hasShadow = hasShadow;
			pl.ubo.samples = samples;
			pl.ubo.radius = radius;
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
