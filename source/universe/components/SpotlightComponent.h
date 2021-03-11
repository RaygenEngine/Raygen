#pragma once
#include "universe/BasicComponent.h"
#include "universe/ComponentsDb.h"
#include "universe/SceneComponentBase.h"
#include "universe/components/LightComponentBase.h"

struct SceneSpotlight;

// TODO:
struct CSpotlight : CLightBase {
	REFLECTED_SCENE_COMP(CSpotlight, SceneSpotlight)
	{
		REFLECT_ICON(FA_LIGHTBULB);
		// REFLECT_CATEGORY("Rendering");

		REFLECT_VAR(color, PropertyFlags::Color).Clamp();
		REFLECT_VAR(intensity).Clamp();
		REFLECT_VAR(near).Clamp(0.001f);
		REFLECT_VAR(far).Clamp(0.001f);
		REFLECT_VAR(shadowMapWidth).Clamp(1.f);
		REFLECT_VAR(shadowMapHeight).Clamp(1.f);
		REFLECT_VAR(maxShadowBias).Clamp(0.001f);
		REFLECT_VAR(samples).Clamp(1.f);
		REFLECT_VAR(radius).Clamp();

		REFLECT_VAR(outerAperture, PropertyFlags::Rads).Clamp(0.001f);
		REFLECT_VAR(innerAperture, PropertyFlags::Rads).Clamp(0.001f);

		REFLECT_VAR(constantTerm).Clamp(1.f);
		REFLECT_VAR(linearTerm).Clamp();
		REFLECT_VAR(quadraticTerm).Clamp();

		REFLECT_VAR(hasShadow);
	}

	CSpotlight();
	~CSpotlight();

	float constantTerm{ 1.f };
	float linearTerm{ 1.f };
	float quadraticTerm{ 1.f };

	// angle
	float outerAperture{ glm::radians(45.f) };
	// inner
	float innerAperture{ glm::radians(22.5f) };

	__declspec(align(16)) struct AlignedData {
		XMMATRIX view;
		XMMATRIX proj;
	};
	AlignedData* pData;
};
