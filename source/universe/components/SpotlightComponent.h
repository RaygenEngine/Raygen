#pragma once
#include "universe/BasicComponent.h"
#include "universe/ComponentsDb.h"
#include "universe/SceneComponentBase.h"

#include "universe/components/LightComponentBase.h"

struct SceneSpotlight;

struct CSpotlight : CLightBase {
	REFLECTED_SCENE_COMP(CSpotlight, SceneSpotlight)
	{
		REFLECT_ICON(FA_LIGHTBULB);
		// REFLECT_CATEGORY("Rendering");

		REFLECT_VAR(color, PropertyFlags::Color);
		REFLECT_VAR(intensity);
		REFLECT_VAR(near_);
		REFLECT_VAR(far_);
		REFLECT_VAR(shadowMapWidth);
		REFLECT_VAR(shadowMapHeight);
		REFLECT_VAR(maxShadowBias);
		REFLECT_VAR(samples);
		REFLECT_VAR(sampleInvSpread);

		REFLECT_VAR(outerAperture, PropertyFlags::Rads);
		REFLECT_VAR(innerAperture, PropertyFlags::Rads);

		REFLECT_VAR(constantTerm);
		REFLECT_VAR(linearTerm);
		REFLECT_VAR(quadraticTerm);

		REFLECT_VAR(hasShadow);
	}

	float constantTerm{ 1.f };
	float linearTerm{ 1.f };
	float quadraticTerm{ 1.f };

	// angle
	float outerAperture{ glm::radians(45.f) };
	// inner
	float innerAperture{ glm::radians(22.5f) };

private:
	glm::mat4 projection;
};
