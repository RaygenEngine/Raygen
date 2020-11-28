#pragma once
#include "universe/BasicComponent.h"
#include "universe/ComponentsDb.h"
#include "universe/SceneComponentBase.h"
#include "universe/components/LightComponentBase.h"

struct ScenePointlight;

struct CPointlight : CLightBase {
	REFLECTED_SCENE_COMP(CPointlight, ScenePointlight)
	{
		REFLECT_ICON(FA_SUN);
		// REFLECT_CATEGORY("Rendering");

		REFLECT_VAR(color, PropertyFlags::Color).Clamp();
		REFLECT_VAR(intensity).Clamp();

		REFLECT_VAR(constantTerm).Clamp(1.f);
		REFLECT_VAR(linearTerm).Clamp();
		REFLECT_VAR(quadraticTerm).Clamp();

		REFLECT_VAR(radius).Clamp();

		REFLECT_VAR(samples).Clamp(1.f);
		REFLECT_VAR(hasShadow);
	}

	float constantTerm{ 1.f };
	float linearTerm{ 1.f };
	float quadraticTerm{ 1.f };

	float CalculateEffectiveRadius() const;
};
