#pragma once
#include "universe/BasicComponent.h"
#include "universe/ComponentsDb.h"
#include "universe/SceneComponentBase.h"

#include "universe/components/LightComponentBase.h"

struct ScenePointlight;

struct CPointlight : CLightBase {
	REFLECTED_SCENE_COMP(CPointlight, ScenePointlight)
	{
		REFLECT_ICON(FA_CIRCLE);
		// REFLECT_CATEGORY("Rendering");

		REFLECT_VAR(color, PropertyFlags::Color).Clamp();
		REFLECT_VAR(intensity).Clamp();

		REFLECT_VAR(constantTerm);
		REFLECT_VAR(linearTerm);
		REFLECT_VAR(quadraticTerm);

		REFLECT_VAR(radius);

		REFLECT_VAR(samples);
		REFLECT_VAR(hasShadow);
	}

	float constantTerm{ 1.f };
	float linearTerm{ 1.f };
	float quadraticTerm{ 1.f };

	float CalculateEffectiveRadius() const;
};
