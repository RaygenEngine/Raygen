#pragma once
#include "assets/AssetPod.h"
#include "assets/util/DynamicDescriptorSet.h"

struct MaterialInstance : AssetPod {

	REFLECTED_POD(MaterialInstance)
	{
		REFLECT_ICON(FA_SWATCHBOOK);
		REFLECT_VAR(archetype, PropertyFlags::Hidden);
	}

	PodHandle<MaterialArchetype> archetype;

	DynamicDescriptorSet descriptorSet;

	// Returns true if something was edited. (property found & type matched)
	template<typename T>
	bool SetUboParameter(std::string_view name, const T& value)
	{
		return archetype.Lock()->descriptorSetLayout.uboClass.SetPropertyValueByName(
			descriptorSet.uboData, name, value);
	}

	void Export(const fs::path& path);
};
