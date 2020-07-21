#include "pch.h"
#include "MaterialInstanceImporter.h"

#include "assets/AssetImporterManager.h"
#include "assets/pods/MaterialInstance.h"
#include "assets/pods/MaterialArchetype.h"
#include "assets/PodEditor.h"
#include "reflection/ReflectionTools.h"

#include <nlohmann/json.hpp>
#include <fstream>


BasePodHandle MaterialInstanceImporter::Import(const fs::path& path)
{

	using namespace nlohmann;

	std::ifstream f(path);

	json j;
	f >> j;

	std::string archPath;
	auto it = j.find("archetype_path");
	if (it != j.end()) {
		it->get_to<std::string>(archPath);
	}
	else {
		LOG_ERROR("MaterialInstanceImporter: archetype attribute not found {}. Skipped importing.", path);
		return {};
	}

	auto archetype = AssetImporterManager->ImportFromMaybeRelative<MaterialArchetype>(archPath, path);

	if (archetype.IsDefault() || !archetype.HasBeenAssigned()) {
		LOG_ERROR("MaterialInstanceImporter: Archetype not found at path: {}. Skipped importing.", archPath);
		return {};
	}

	auto& [handle, pod] = AssetImporterManager->CreateEntry<MaterialInstance>(
		path.generic_string(), "INST " + path.filename().replace_extension().generic_string(), false, true);

	{
		// CHECK: Editor in importer
		PodEditor arch(archetype);
		arch->instances.push_back(handle);
	}

	auto archPod = archetype.Lock();
	pod->archetype = archetype;
	pod->descriptorSet.SwapLayout({}, archPod->descriptorSetLayout);


	if (it = j.find("ubo_object"); it != j.end()) {
		refltools::JsonToPropVisitor visitor(*it);

		refltools::CallVisitorOnEveryPropertyEx(
			pod->descriptorSet.uboData.data(), archPod->descriptorSetLayout.uboClass, visitor);
	}
	else {
		LOG_WARN("MaterialInstanceImporter: Missing ubo_object property in json: {}", path);
	}


	if (it = j.find("samplers2d"); it != j.end()) {
		if (!it->is_object()) {
			LOG_ERROR("MaterialInstanceImporter: Samplers2d not an object in json: {}");
		}
		else {
			for (int32 samplerIndex = 0; auto& name : archPod->descriptorSetLayout.samplers2d) {
				auto samplerIt = it->find(name);
				if (samplerIt != it->end()) {
					pod->descriptorSet.samplers2d[samplerIndex]
						= AssetImporterManager->ImportOrFindFromJson<Image>(*samplerIt, path);
				}
				samplerIndex++;
			}
		}
	}
	else {
		LOG_WARN("MaterialInstanceImporter: Missing samplers2d property in json: {}", path);
	}

	return handle;
}
