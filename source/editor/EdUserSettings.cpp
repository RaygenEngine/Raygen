#include "pch/pch.h"
#include "editor/EdUserSettings.h"
#include "system/Logger.h"
#include "reflection/ReflectionTools.h"
#include <nlohmann/json.hpp>


namespace ed {
UserSettings UserSettings::Load(fs::path filename)
{
	using namespace nlohmann;
	UserSettings inst{};
	inst.loadedFrom = filename;

	refltools::FromJsonFile(&inst, filename);

	return inst;
}
void UserSettings::Save(const fs::path& filename, UserSettings& settings)
{
	if (!refltools::ToJsonFile(&settings, filename)) {
		LOG_ERROR("Failed to save editor user settings at: {}", filename);
	}
}
} // namespace ed
