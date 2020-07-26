#pragma once
#include "reflection/GenMacros.h"

namespace ed {

// Editor user settings.
// Maybe should be made generic user settings
struct UserSettings {
	REFLECTED_GENERIC(UserSettings) { REFLECT_VAR(openWindows); }

	// User Settings variables
	std::vector<std::string> openWindows;

	//
	fs::path loadedFrom{};
	UserSettings() = default;
	bool dirty{ false };


	//
	// Function utilities
	//
	static UserSettings Load(fs::path filename);
	static void Save(const fs::path& filename, UserSettings& settings);

	void Save(const fs::path& filename) { UserSettings::Save(filename, *this); }
	void Reload() { (*this) = UserSettings::Load(loadedFrom); }

	void SaveIfDirty()
	{
		if (dirty) {
			Save(loadedFrom);
			dirty = false;
		}
	}

	void MarkDirty() { dirty = true; }

	// Explicit copy
	UserSettings GetCopy() { return *this; };
	UserSettings(UserSettings&& other) = default;
	UserSettings& operator=(UserSettings&& other) = default;

protected:
	// Should be used explicitly
	UserSettings(const UserSettings& other) = default;
	UserSettings& operator=(const UserSettings& other) = default;
};

// Singleton like access for user settings with hardcoded file
inline UserSettings& GetSettings()
{
	static UserSettings instance = UserSettings::Load("EditorUserSettings.json");
	return instance;
}
} // namespace ed
