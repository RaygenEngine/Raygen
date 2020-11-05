#include "SpotlightComponent.h"

#include "rendering/scene/SceneSpotlight.h"

DECLARE_DIRTY_FUNC(CSpotlight)(BasicComponent& bc)
{
	auto lookAt = bc.world().position + bc.world().front();
	auto view = glm::lookAt(bc.world().position, lookAt, bc.world().up());

	[[maybe_unused]] float outerCutOff;
	[[maybe_unused]] float innerCutOff;

	if constexpr (FullDirty) {
		outerCutOff = glm::cos(outerAperture / 2.f);
		innerCutOff = glm::cos(innerAperture / 2.f);

		const auto ar = static_cast<float>(shadowMapWidth) / static_cast<float>(shadowMapHeight);
		proj = glm::perspective(outerAperture, ar, _near, _far);
		// Vulkan's inverted y
		proj[1][1] *= -1.f;
	}

	glm::mat4 viewProj = proj * view;

	return [=](SceneSpotlight& sl) {
		sl.name = "spot depth: " + bc.name;
		sl.ubo.position = glm::vec4(bc.world().position, 1.f);
		sl.ubo.front = glm::vec4(bc.world().front(), 0.f);
		sl.ubo.viewProj = viewProj;

		if constexpr (FullDirty) {
			sl.ubo.color = glm::vec4(color, 1.f);
			sl.ubo.intensity = intensity;
			sl.ubo._near = _near;
			sl.ubo._far = _far;
			sl.ubo.outerCutOff = outerCutOff;
			sl.ubo.innerCutOff = innerCutOff;
			sl.ubo.constantTerm = constantTerm;
			sl.ubo.linearTerm = linearTerm;
			sl.ubo.quadraticTerm = quadraticTerm;
			sl.ubo.maxShadowBias = maxShadowBias;
			sl.ubo.samples = samples;
			sl.ubo.sampleInvSpread = radius;

			sl.MaybeResizeShadowmap(shadowMapWidth, shadowMapHeight);
		}
	};
}
