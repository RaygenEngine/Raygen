#pragma once
#include "assets/AssetPod.h"
#include "assets/util/DynamicDescriptorSet.h"

struct MaterialInstance : AssetPod {

	REFLECTED_POD(MaterialInstance)
	{
		REFLECT_ICON(FA_SWATCHBOOK);
		REFLECT_VAR(archetype);
	}

	PodHandle<MaterialArchetype> archetype;

	DynamicDescriptorSet descriptorSet;
};
