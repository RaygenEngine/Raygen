#include "EdUserSettings.h"

#include "reflection/ReflectionTools.h"

namespace ed {
UserSettings UserSettings::Load(fs::path filename)
{
	using namespace nlohmann;
	UserSettings inst{};
	inst.loadedFrom = filename;

	if (fs::exists(filename)) {
		refltools::FromJsonFile(&inst, filename);
	}
	else {
		inst.openWindows = { "Outliner", "Asset Browser", "Property Editor", "Console" };
		inst.MarkDirty();
	}


	return inst;
}
void UserSettings::Save(const fs::path& filename, UserSettings& settings)
{
	if (!refltools::ToJsonFile(&settings, filename)) {
		LOG_ERROR("Failed to save editor user settings at: {}", filename);
	}
}
} // namespace ed
