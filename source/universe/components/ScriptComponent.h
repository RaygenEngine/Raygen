#pragma once
#include "universe/BasicComponent.h"
#include "universe/ComponentsDb.h"
#include "universe/SceneComponentBase.h"

struct CScript {
	// WIP: Tickable, Begin, End play detection
	COMP_DIRTABLE;
	COMP_CREATEDESTROY;
	COMP_TICKABLE;
	COMP_BEGINENDPLAY;

	REFLECTED_COMP(CScript)
	{
		REFLECT_ICON(FA_PRESCRIPTION);

		REFLECT_CATEGORY("Gameplay");
		REFLECT_VAR(movementSpeed);
	}

	float movementSpeed{ 1.f };

	void BeginPlay();
	void EndPlay();
	void Tick(float deltaSeconds);

	Entity self; // WIP: Begin End play at construction/destruction
};
