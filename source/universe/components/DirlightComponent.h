#pragma once

#include "universe/ComponentsDb.h"
#include "universe/components/LightComponentBase.h"


struct SceneDirlight;

// TODO:
struct CDirlight : CLightBase {

	REFLECTED_SCENE_COMP(CDirlight, SceneDirlight)
	{
		REFLECT_ICON(FA_LOCATION_ARROW);

		REFLECT_VAR(color, PropertyFlags::Color).Clamp();
		REFLECT_VAR(intensity).Clamp();
		REFLECT_VAR(near).Clamp(0.001f);
		REFLECT_VAR(far).Clamp(0.001f);
		REFLECT_VAR(shadowMapWidth).Clamp(1.f);
		REFLECT_VAR(shadowMapHeight).Clamp(1.f);
		REFLECT_VAR(maxShadowBias).Clamp(0.001f);
		REFLECT_VAR(samples).Clamp(1.f);
		REFLECT_VAR(radius).Clamp();

		REFLECT_VAR(hasShadow);


		REFLECT_VAR(left);
		REFLECT_VAR(right);
		REFLECT_VAR(top);
		REFLECT_VAR(bottom);
	}

	CDirlight();
	~CDirlight();

	float left{ -20.f };
	float right{ 20.f };

	float bottom{ -20.f };
	float top{ 20.f };

	__declspec(align(16)) struct AlignedData {
		XMMATRIX view;
		XMMATRIX proj;
	};
	AlignedData* pData;
};
