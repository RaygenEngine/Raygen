#pragma once
#include "universe/BasicComponent.h"
#include "universe/ComponentsDb.h"
#include "universe/SceneComponentBase.h"

struct SceneIrradianceGrid;

struct CIrradianceGrid : CSceneBase {
	REFLECTED_SCENE_COMP(CIrradianceGrid, SceneIrradianceGrid)
	{
		// REFLECT_ICON(FA_CUBE);
		// REFLECT_CATEGORY("Rendering");
		REFLECT_VAR(distToAdjacent);
		REFLECT_VAR(ptSamples);
		REFLECT_VAR(ptBounces);
		REFLECT_VAR(notifyBuild, PropertyFlags::Transient);
		REFLECT_VAR(hideBillboards, PropertyFlags::Transient);
	}


	float distToAdjacent{ 1.f };
	int32 ptSamples{ 2 };
	int32 ptBounces{ 2 };

	bool notifyBuild{ true };
	bool hideBillboards{ false };
};
