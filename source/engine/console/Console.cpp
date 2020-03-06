#include "engine/console/Console.h"
#include "engine/console/ConsoleVariable.h"
#include "core/StringUtl.h"

ConsoleFunctionGeneric commandAll(
	"all",
	[](std::string_view view) {
		auto parts = str::split(view);

		if (parts.size() <= 1) {
			for (auto& [key, entry] : Console::Z_GetEntries()) {
				LOG_REPORT("{:<30} - {}", key, entry->tooltip);
			}
			return;
		}

		auto filter = parts[1];

		for (auto& [key, entry] : Console::Z_GetEntries()) {
			if (str::startsWithInsensitive(key, filter)) {
				LOG_REPORT("{:<30} - {}", key, entry->tooltip);
			}
		}
	},
	"Lists all available commands [starting with the text passed as param]");

void Console::Execute(const std::string& command)
{
	Execute(std::string_view(command));
}

void Console::Execute(std::string_view command)
{
	auto parts = str::split(command);
	auto& entries = Get().m_entries;

	auto it = entries.find(parts[0]);
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

auto Console::Z_GetEntries() -> decltype(m_entries)&
{
	return Get().m_entries;
}
