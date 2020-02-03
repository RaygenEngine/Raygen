#include "system/console/Console.h"
#include "system/console/ConsoleVariable.h"

void Console::Execute(const std::string& command)
{
	Execute(std::string_view(command));
}

void Console::Execute(std::string_view command)
{
	if (command.starts_with("all")) {
		for (auto& [key, z] : Get().m_entries) {
			LOG_REPORT("{}", key);
		}
		return;
	}
	auto parts = str::Split(command);
	auto& entries = Get().m_entries;

	auto it = entries.find(std::string(parts[0]));
	if (it != entries.end()) {
		it->second->Execute(command);
		return;
	}
	LOG_REPORT("Command {} not found.", parts[0]);
}
