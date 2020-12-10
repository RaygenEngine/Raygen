#pragma once
#include "universe/BasicComponent.h"
#include "universe/ComponentsDb.h"
#include "universe/SceneComponentBase.h"
#include "universe/components/LightComponentBase.h"

struct SceneQuadlight;

struct CQuadlight : CLightBase {
	REFLECTED_SCENE_COMP(CQuadlight, SceneQuadlight)
	{
		REFLECT_ICON(FA_SQUARE_FULL);
		// REFLECT_CATEGORY("Rendering");

		REFLECT_VAR(color, PropertyFlags::Color).Clamp();
		REFLECT_VAR(intensity).Clamp();

		REFLECT_VAR(width).Clamp();
		REFLECT_VAR(height).Clamp();

		REFLECT_VAR(aperture, PropertyFlags::Rads).Clamp(0.001f);

		REFLECT_VAR(constantTerm).Clamp(1.f);
		REFLECT_VAR(linearTerm).Clamp();
		REFLECT_VAR(quadraticTerm).Clamp();

		REFLECT_VAR(samples).Clamp(1.f);
		REFLECT_VAR(hasShadow);
	}

	float width{ 0.5f };
	float height{ 0.5f };

	float aperture{ glm::radians(90.f) };

	float constantTerm{ 1.f };
	float linearTerm{ 1.f };
	float quadraticTerm{ 1.f };
};
