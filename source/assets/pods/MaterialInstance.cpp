#include "pch.h"
#include "MaterialInstance.h"


#include "assets/pods/MaterialArchetype.h"
#include "reflection/ReflectionTools.h"

#include <nlohmann/json.hpp>
#include <fstream>

void MaterialInstance::Export(const fs::path& path)
{
	using namespace nlohmann;


	json j;

	auto archPod = archetype.Lock();

	auto& cl = archPod->descriptorSetLayout.uboClass;


	// Fill archetype_path
	AssetHandlerManager::GenerateRelativeExportJsonObject(j["archetype_path"], path, archetype);

	// Fill ubo_object
	j["ubo_object"] = json::object();
	refltools::ToJsonVisitor visitor(j["ubo_object"]);
	refltools::CallVisitorOnEveryPropertyEx(descriptorSet.uboData.data(), cl, visitor);

	// Fill samplers2d

	auto& jsonSamplers = j["samplers2d"];
	jsonSamplers = json::object();

	for (int32 samplerIndex = 0; auto& samplerName : archPod->descriptorSetLayout.samplers2d) {
		AssetHandlerManager::GenerateRelativeExportJsonObject(
			jsonSamplers[samplerName], path, descriptorSet.samplers2d[samplerIndex]);
	}

	std::ofstream file(path);
	file << std::setw(4) << j;
}
