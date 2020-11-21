#include "DirlightComponent.h"

#include "assets/PodEditor.h"
#include "assets/pods/MaterialArchetype.h"
#include "assets/pods/MaterialInstance.h"
#include "rendering/scene/SceneDirlight.h"


DECLARE_DIRTY_FUNC(CDirlight)(BasicComponent& bc)
{
	if (!skyInstance.IsDefault()) {
		PodEditor editor(skyInstance);
		editor->SetUboParameter("sunDirection", glm::vec4(bc.world().front(), 0.f));
		editor->SetUboParameter("sunColor", glm::vec4(color, 1.f));
		editor->SetUboParameter("sunIntensity", intensity);
	}

	auto lookAt = bc.world().position + bc.world().front();
	auto view = glm::lookAt(bc.world().position, lookAt, bc.world().up());

	if constexpr (FullDirty) {
		proj = glm::ortho(left, right, bottom, top, _near, _far);
		// Vulkan's inverted y
		proj[1][1] *= -1.f;
	}


	glm::mat4 viewProj = proj * view;

	return [=](SceneDirlight& dl) {
		dl.name = "direct depth: " + bc.name;

		dl.up = bc.world().up();
		dl.ubo.front = glm::vec4(bc.world().front(), 0.f);
		dl.ubo.viewProj = viewProj;

		if constexpr (FullDirty) {
			dl.ubo.color = glm::vec4(color, 1.f);
			dl.ubo.intensity = intensity;

			dl.ubo.maxShadowBias = maxShadowBias;
			dl.ubo.samples = samples;
			dl.ubo.sampleInvSpread = 1 / radius;
			dl.ubo.hasShadow = hasShadow;

			dl.MaybeResizeShadowmap(shadowMapWidth, shadowMapHeight);
		}
	};
}
