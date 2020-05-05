#pragma once
#include "core/StringUtl.h"
#include <unordered_set>

// Dummy struct which can be used in the future to pass generic update info from cpu to gpu asset updating
// (This could for example contain a hashset of the parameters that got changed for a specific type)
struct AssetUpdateInfo {
	std::unordered_set<std::string, str::HashInsensitive> flags;

	void AddFlag(const std::string& str) { flags.insert(str); }
	void AddFlag(const char* str) { flags.insert(str); }

	[[nodiscard]] bool HasFlag(const char* str) const { return flags.count(str) > 0; }
	[[nodiscard]] bool HasFlag(const std::string& str) const { return flags.count(str) > 0; }
};
