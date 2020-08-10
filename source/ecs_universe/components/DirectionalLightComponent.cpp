#include "pch.h"
#include "DirectionalLightComponent.h"

#include "assets/PodEditor.h"
#include "assets/pods/MaterialArchetype.h"
#include "assets/pods/MaterialInstance.h"
#include "rendering/scene/SceneDirectionalLight.h"


DECLARE_DIRTY_FUNC(CDirectionalLight)(BasicComponent& bc)
{
	if (!skyInstance.IsDefault()) {
		PodEditor editor(skyInstance);
		editor->SetUboParameter("sunDirection", glm::vec4(bc.world().forward(), 0.f));
		editor->SetUboParameter("sunColor", glm::vec4(color, 1.f));
		editor->SetUboParameter("sunIntensity", intensity);
	}

	auto lookAt = bc.world().position + bc.world().forward();
	auto view = glm::lookAt(bc.world().position, lookAt, bc.world().up());

	if constexpr (FullDirty) {
		proj = glm::ortho(left, right, bottom, top, near_, far_);
		// Vulkan's inverted y
		proj[1][1] *= -1.f;
	}


	glm::mat4 viewProj = proj * view;

	return [=](SceneDirectionalLight& dl) {
		dl.name = "direct depth: " + bc.name;

		dl.up = bc.world().up();
		dl.ubo.forward = glm::vec4(bc.world().forward(), 0.f);
		dl.ubo.viewProj = viewProj;

		if constexpr (FullDirty) {
			dl.ubo.color = glm::vec4(color, 1.f);
			dl.ubo.intensity = intensity;

			dl.ubo.maxShadowBias = maxShadowBias;
			dl.ubo.samples = samples;
			dl.ubo.sampleInvSpread = sampleInvSpread;

			dl.ResizeShadowmap(shadowMapWidth, shadowMapHeight);
		}
	};
}
