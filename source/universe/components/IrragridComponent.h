#pragma once
#include "universe/BasicComponent.h"
#include "universe/ComponentsDb.h"
#include "universe/SceneComponentBase.h"

struct SceneIrragrid;

struct CIrragrid : CSceneBase {
	REFLECTED_SCENE_COMP(CIrragrid, SceneIrragrid)
	{
		// REFLECT_ICON(FA_CUBE);
		// REFLECT_CATEGORY("Rendering");

		REFLECT_VAR(width);
		REFLECT_VAR(height);
		REFLECT_VAR(depth);

		REFLECT_VAR(distToAdjacent);

		REFLECT_VAR(ptSamples).Clamp(1);
		REFLECT_VAR(ptBounces);

		REFLECT_VAR(irrResolution);

		REFLECT_VAR(notifyBuild, PropertyFlags::Transient);
		REFLECT_VAR(hideBillboards, PropertyFlags::Transient);
	}

	int32 width{ 3 };
	int32 height{ 3 };
	int32 depth{ 3 };

	float distToAdjacent{ 1.f };

	int32 ptSamples{ 2 };
	int32 ptBounces{ 2 };

	int32 irrResolution{ 32 };

	bool notifyBuild{ true };
	bool hideBillboards{ false };
};
