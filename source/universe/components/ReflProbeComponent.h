#pragma once
#include "universe/BasicComponent.h"
#include "universe/ComponentsDb.h"
#include "universe/SceneComponentBase.h"

struct SceneReflprobe;

struct CReflprobe : CSceneBase {
	REFLECTED_SCENE_COMP(CReflprobe, SceneReflprobe)
	{
		// REFLECT_ICON(FA_CUBE);
		// REFLECT_CATEGORY("Rendering");
		REFLECT_VAR(radius).Clamp();

		REFLECT_VAR(ptSamples).Clamp(1.f);
		REFLECT_VAR(ptBounces).Clamp(0.f);

		REFLECT_VAR(irrResolution).Clamp(1.f);

		REFLECT_VAR(prefLodCount).Clamp(1.f, 5.f);
		REFLECT_VAR(prefSamples).Clamp(1.f);

		REFLECT_VAR(applyIrradiance);
		REFLECT_VAR(notifyBuild, PropertyFlags::Transient);
	}

	float radius{ 1.5f };

	int32 ptSamples{ 32 };
	int32 ptBounces{ 1 };

	int32 irrResolution{ 32 };

	int32 prefLodCount{ 3 };
	int32 prefSamples{ 1024 };

	bool applyIrradiance{ true };
	bool notifyBuild{ true };
};
