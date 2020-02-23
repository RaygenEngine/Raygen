#pragma once
#include "core/StringAux.h"
#include "system/Logger.h"
#include "core/StringHashing.h"
#include <unordered_map>
#include <string>

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
		return *instance;
	}

	std::unordered_map<std::string, ConsoleEntry*, str::HashInsensitive, str::EqualInsensitive> m_entries;

public:
	// TODO: allow only ConsoleVariable
	static void AutoRegister(const char* name, ConsoleEntry* entry)
	{
		auto& c = Get();
		if (!c.m_entries.emplace(name, entry).second) {
			Log::EarlyInit(); // Required since this may be called before main
			LOG_ERROR("Failed to register a duplicate console variable: {}", name);
		}
	}

	static void Execute(const std::string& command);
	static void Execute(std::string_view command);

	// BOTH must match for safety reasons. (avoid removing collision)
	static void Unregister(const std::string& command, ConsoleEntry* entry)
	{
		auto& entries = Get().m_entries;
		auto it = entries.find(command);

		if (it != entries.end() && it->second == entry) {
			entries.erase(command);
		}
	}

	static std::vector<ConsoleEntry*> AutoCompleteSuggest(std::string_view currentPart);

	static decltype(m_entries)& Z_GetEntries();
};
