#pragma once

#include "core/StringUtl.h"
#include "engine/Logger.h"

#include <string>
#include <unordered_map>

struct ConsoleEntry;

class Console {
private:
	Console() = default; // Cannot register commands in this constructor

protected:
	Console(const Console&) = delete;
	Console(Console&&) = delete;
	Console operator=(const Console&) = delete;
	Console operator=(Console&&) = delete;


	[[nodiscard]] static Console& Get()
	{
		static Console* instance = new Console(); // Avoid de initialisation errors.
		return *instance;                         // ConsoleVars auto unregister on destruction and may be global
	}

	std::unordered_map<std::string, ConsoleEntry*, str::HashInsensitive, str::EqualInsensitive> m_entries;

public:
	static void AutoRegister(const char* name, ConsoleEntry* entry);


	static void Execute(const std::string& command);
	static void Execute(std::string_view command);

	// BOTH must match for safety reasons. (avoid removing collision)
	static void Unregister(const std::string& command, ConsoleEntry* entry);

	[[nodiscard]] static std::vector<ConsoleEntry*> AutoCompleteSuggest(std::string_view currentPart);

	[[nodiscard]] static auto& Z_GetEntries() { return Get().m_entries; }
};
