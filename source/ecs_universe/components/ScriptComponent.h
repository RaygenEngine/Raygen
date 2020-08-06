#pragma once
#include "ecs_universe/BasicComponent.h"
#include "ecs_universe/ComponentsDb.h"
#include "ecs_universe/SceneComponentBase.h"

struct CScript {
	COMP_DIRTABLE;
	COMP_CREATEDESTROY;
	REFLECTED_COMP(CScript)
	{
		REFLECT_ICON(FA_PRESCRIPTION);
		REFLECT_CATEGORY("Gameplay");
		REFLECT_VAR(code);
	}

	std::string code;
};
