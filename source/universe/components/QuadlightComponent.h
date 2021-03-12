#pragma once
#include "universe/BasicComponent.h"
#include "universe/ComponentsDb.h"
#include "universe/components/LightComponentBase.h"

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
	}

	float width{ 0.5f };
	float height{ 0.5f };

	float aperture{ glm::radians(90.f) };
};
