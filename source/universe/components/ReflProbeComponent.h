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
		REFLECT_VAR(innerRadius);
		REFLECT_VAR(outerRadius);

		REFLECT_VAR(ptSamples);
		REFLECT_VAR(ptBounces);

		REFLECT_VAR(irrResolution);

		REFLECT_VAR(prefLodCount);
		REFLECT_VAR(prefSamples);

		REFLECT_VAR(notifyBuild, PropertyFlags::Transient);
	}

	float innerRadius{ 1.5f };
	float outerRadius{ 70.f };

	int32 ptSamples{ 16 };
	int32 ptBounces{ 3 };

	int32 irrResolution{ 32 };

	int32 prefLodCount{ 5 };
	int32 prefSamples{ 1024 };

	bool notifyBuild{ true };
};
