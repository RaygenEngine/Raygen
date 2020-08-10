#pragma once

#include "ecs_universe/ComponentsDb.h"
#include "ecs_universe/components/LightComponentBase.h"

struct SceneDirectionalLight;

struct CDirectionalLight : CLightBase {

	REFLECTED_SCENE_COMP(CDirectionalLight, SceneDirectionalLight)
	{
		REFLECT_ICON(FA_LIGHTBULB);

		REFLECT_VAR(color, PropertyFlags::Color);
		REFLECT_VAR(intensity);
		REFLECT_VAR(near_);
		REFLECT_VAR(far_);
		REFLECT_VAR(shadowMapWidth);
		REFLECT_VAR(shadowMapHeight);
		REFLECT_VAR(maxShadowBias);
		REFLECT_VAR(samples);
		REFLECT_VAR(sampleInvSpread);

		REFLECT_VAR(hasShadow);


		REFLECT_VAR(left);
		REFLECT_VAR(right);
		REFLECT_VAR(top);
		REFLECT_VAR(bottom);

		REFLECT_VAR(skyInstance);
	}

	float left{ -20.f };
	float right{ 20.f };

	float bottom{ -20.f };
	float top{ 20.f };

	PodHandle<MaterialInstance> skyInstance;

private:
	glm::mat4 proj;
};
