#pragma once
#include "universe/BasicComponent.h"
#include "universe/ComponentsDb.h"
#include "universe/SceneComponentBase.h"

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
