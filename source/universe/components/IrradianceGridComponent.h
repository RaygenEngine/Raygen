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
		REFLECT_VAR(blendProportion);
		REFLECT_VAR(notifyBuild, PropertyFlags::Transient);
		REFLECT_VAR(hideBillboards, PropertyFlags::Transient);
	}


	float distToAdjacent{ 1.f };
	float blendProportion{ 0.2f };

	bool notifyBuild{ true };
	bool hideBillboards{ false };
};
