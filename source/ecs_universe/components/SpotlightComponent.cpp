#include "pch.h"
#include "SpotlightComponent.h"

#include "rendering/scene/SceneSpotlight.h"

DECLARE_DIRTY_FUNC(CSpotlight)(BasicComponent& bc)
{

	auto lookAt = bc.world().position + bc.world().forward();
	auto view = glm::lookAt(bc.world().position, lookAt, bc.world().up());

	[[maybe_unused]] float outerCutOff;
	[[maybe_unused]] float innerCutOff;

	if constexpr (FullDirty) {
		outerCutOff = glm::cos(outerAperture / 2.f);
		innerCutOff = glm::cos(innerAperture / 2.f);

		const auto ar = static_cast<float>(shadowMapWidth) / static_cast<float>(shadowMapHeight);
		projection = glm::perspective(outerAperture, ar, near_, far_);
		// Vulkan's inverted y
		projection[1][1] *= -1.f;
	}

	glm::mat4 viewProj = projection * view;

	return [=](SceneSpotlight& sl) {
		sl.name = "spot depth: " + bc.name;
		sl.ubo.position = glm::vec4(bc.world().position, 1.f);
		sl.ubo.forward = glm::vec4(bc.world().forward(), 0.f);
		sl.ubo.viewProj = viewProj;

		if constexpr (FullDirty) {
			sl.ubo.color = glm::vec4(color, 1.f);
			sl.ubo.intensity = intensity;
			sl.ubo.near_ = near_;
			sl.ubo.far_ = far_;
			sl.ubo.outerCutOff = outerCutOff;
			sl.ubo.innerCutOff = innerCutOff;
			sl.ubo.constantTerm = constantTerm;
			sl.ubo.linearTerm = linearTerm;
			sl.ubo.quadraticTerm = quadraticTerm;
			sl.ubo.maxShadowBias = maxShadowBias;
			sl.ubo.samples = samples;
			sl.ubo.sampleInvSpread = sampleInvSpread;

			sl.MaybeResizeShadowmap(shadowMapWidth, shadowMapHeight);
		}
	};
}
