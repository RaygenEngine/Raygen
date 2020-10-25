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
		// REFLECT_VAR(environmentMap);
		REFLECT_VAR(resolution);
		REFLECT_VAR(shouldBuild);
	}

	// PodHandle<EnvironmentMap> environmentMap;

	float innerRadius;
	float outerRadius;

	bool shouldBuild{ true };

	int32 resolution{ 128 };
};
