#include "pch.h"
#include "Console.h"

#include "core/StringUtl.h"
#include "engine/console/ConsoleVariable.h"

ConsoleFunctionGeneric commandAll(
	"all",
	[](std::string_view view) {
		auto parts = str::split(view);

		if (parts.size() <= 1) {
			for (auto& [key, entry] : Console::Z_GetEntries()) {
				LOG_REPORT("{}", entry->GetDescriptionLine());
			}
			return;
		}

		auto filter = parts[1];

		for (auto& [key, entry] : Console::Z_GetEntries()) {
			if (str::startsWithInsensitive(key, filter)) {
				LOG_REPORT("{}", entry->GetDescriptionLine());
			}
		}
	},
	"Lists all available commands [starting with the text passed as param]");

ConsoleFunction<int32, bool> g("test", [](auto, bool) {});

void Console::Execute(const std::string& command)
{
	Execute(std::string_view(command));
}

void Console::Execute(std::string_view command)
{
	auto parts = str::split(command);
	auto& entries = Get().m_entries;

	// WIP: should be string view
	auto it = entries.find(std::string(parts[0]));
	if (it != entries.end()) {
		it->second->Execute(command);
		return;
	}
	LOG_REPORT("Command {} not found.", parts[0]);
}

std::vector<ConsoleEntry*> Console::AutoCompleteSuggest(std::string_view currentPart)
{
	std::vector<ConsoleEntry*> found;

	for (auto& [key, entry] : Get().m_entries) {
		if (str::startsWithInsensitive(key, currentPart)) {
			found.push_back(entry);
		}
	}
	return found;
}

void Console::AutoRegister(const char* name, ConsoleEntry* entry)
{
	auto& c = Get();
	if (!c.m_entries.emplace(name, entry).second) {
		Log.EarlyInit(); // Required since this may be called before main
		LOG_ERROR("Failed to register a duplicate console variable: {}", name);
	}
}

void Console::Unregister(const std::string& command, ConsoleEntry* entry)
{
	auto& entries = Get().m_entries;
	auto it = entries.find(command);

	if (it != entries.end() && it->second == entry) {
		entries.erase(command);
	}
}
