#pragma once
#include "assets/AssetPod.h"

struct Prefab : AssetPod {

	REFLECTED_POD(Prefab)
	{
		REFLECT_ICON(FA_BOXES);
		REFLECT_VAR(data);
	}

	// Json string of the hierarchy. Avoid using directly, prefer the member functions  where possible
	std::string data{ "{}" };


	void InsertInto(World& into) const;


	// In the future support multiple entities bundled together? or force dummy parent entity for easier "reload from
	// prefab" functions
	void MakeFrom(Entity entity);
};
