#pragma once

#include "universe/ComponentsDb.h"
#include "universe/components/LightComponentBase.h"

struct SceneDirlight;

// TODO:
struct CDirlight : CLightBase {

	REFLECTED_SCENE_COMP(CDirlight, SceneDirlight)
	{
		REFLECT_ICON(FA_LOCATION_ARROW);

		REFLECT_VAR(color, PropertyFlags::Color);
		REFLECT_VAR(intensity);
		REFLECT_VAR(_near);
		REFLECT_VAR(_far);
		REFLECT_VAR(shadowMapWidth);
		REFLECT_VAR(shadowMapHeight);
		REFLECT_VAR(maxShadowBias);
		REFLECT_VAR(samples);
		REFLECT_VAR(radius);

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

	glm::mat4 view;
	glm::mat4 proj;
};
