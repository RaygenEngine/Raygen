#pragma once
#include "universe/BasicComponent.h"
#include "universe/ComponentsDb.h"
#include "universe/SceneComponentBase.h"

struct SceneReflProbe;

struct CReflProbe : CSceneBase {
	REFLECTED_SCENE_COMP(CReflProbe, SceneReflProbe)
	{
		// REFLECT_ICON(FA_CUBE);
		// REFLECT_CATEGORY("Rendering");

		REFLECT_VAR(environmentMap);
	}

	PodHandle<EnvironmentMap> environmentMap;
};
