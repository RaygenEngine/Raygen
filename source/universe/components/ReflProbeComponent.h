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
		REFLECT_VAR(resolution);
		REFLECT_VAR(shouldBuild);
	}


	float innerRadius{ 1.5f };
	float outerRadius{ 70.f };

	int32 ptSamples{ 16u };
	int32 ptBounces{ 3u };

	int32 resolution{ 128 };

	bool shouldBuild{ true };
};
