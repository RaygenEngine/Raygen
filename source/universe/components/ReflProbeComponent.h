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

		REFLECT_VAR(environmentMap);
	}

	PodHandle<EnvironmentMap> environmentMap;
};
