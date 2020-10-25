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
		REFLECT_VAR(environmentMap);
	}

	PodHandle<EnvironmentMap> environmentMap;

	float innerRadius{ 10.f };
	float outerRadius{ 15.f };
};
