#pragma once
#include "universe/BasicComponent.h"
#include "universe/ComponentsDb.h"
#include "universe/SceneComponentBase.h"

struct SceneReflectionProbe;

struct CReflectionProbe : CSceneBase {
	REFLECTED_SCENE_COMP(CReflectionProbe, SceneReflectionProbe)
	{
		// REFLECT_ICON(FA_CUBE);
		// REFLECT_CATEGORY("Rendering");

		REFLECT_VAR(environmentMap);
	}

	PodHandle<EnvironmentMap> environmentMap;
};
