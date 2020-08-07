#pragma once
#include "ecs_universe/BasicComponent.h"
#include "ecs_universe/ComponentsDb.h"
#include "ecs_universe/SceneComponentBase.h"

struct SceneSpotlight;

struct CSpotlight : CSceneBase {
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

	glm::vec3 color{ 1.f };
	float intensity{ 30.f };

	bool hasShadow{ true };

	int32 shadowMapWidth{ 2048 };
	int32 shadowMapHeight{ 2048 };

	float near_{ 0.05f };
	float far_{ 20.0f };

	float maxShadowBias{ 0.005f };
	int32 samples{ 4 };
	float sampleInvSpread{ 1000.f };

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
