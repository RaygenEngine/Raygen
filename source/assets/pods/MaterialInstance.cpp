#include "MaterialInstance.h"


#include "reflection/ReflectionTools.h"
#include "assets/PodEditor.h"


void MaterialInstance::Export(const fs::path& path)
{
	using namespace nlohmann;


	json j;

	auto archPod = archetype.Lock();

	auto& cl = archPod->descriptorSetLayout.uboClass;


	// Fill archetype_path
	AssetRegistry::GenerateRelativeExportJsonObject(j["archetype_path"], path, archetype);

	// Fill ubo_object
	j["ubo_object"] = json::object();
	refltools::ToJsonVisitor visitor(j["ubo_object"]);
	refltools::CallVisitorOnEveryPropertyEx(descriptorSet.uboData.data(), cl, visitor);

	// Fill samplers2d

	auto& jsonSamplers = j["samplers2d"];
	jsonSamplers = json::object();

	for (int32 samplerIndex = 0; auto& samplerName : archPod->descriptorSetLayout.samplers2d) {
		AssetRegistry::GenerateRelativeExportJsonObject(
			jsonSamplers[samplerName], path, descriptorSet.samplers2d[samplerIndex++]);
	}

	std::ofstream file(path);
	file << std::setw(4) << j;
}

void MaterialInstance::SetArchetype(
	PodHandle<MaterialInstance> instanceHandle, PodHandle<MaterialArchetype> newArchetypeHandle)
{
	PodEditor instance(instanceHandle);

	auto newArch = newArchetypeHandle.Lock();
	auto prevArch = instance->archetype.Lock();

	// Remove from previous arhcetype list
	if (auto it = std::find(prevArch->instances.begin(), prevArch->instances.end(), instanceHandle);
		it != prevArch->instances.end()) {

		PodEditor prevArchEditor(instance->archetype);
		prevArchEditor->instances.erase(it);
	}


	// Add to new arhcetype list
	if (auto it = std::find(newArch->instances.begin(), newArch->instances.end(), instanceHandle);
		it == newArch->instances.end()) {

		PodEditor newArchEd(newArchetypeHandle);
		newArchEd->instances.push_back(instanceHandle);
	}

	instance->descriptorSet.SwapLayout(prevArch->descriptorSetLayout, newArch->descriptorSetLayout);
	instance->archetype = newArchetypeHandle;
}
