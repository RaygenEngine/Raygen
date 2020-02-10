#pragma once
#include "core/StringAux.h"
#include "core/StringConversions.h"
#include "system/console/Console.h"
#include "system/Logger.h"
#include <string_view>

struct ConsoleEntry {
	const char* name;
	const char* tooltip;

	ConsoleEntry(const char* name, const char* tooltip = "")
		: name(name)
		, tooltip(tooltip)
	{
		Console::AutoRegister(name, this);
	}

	virtual void Execute(std::string_view command) { LOG_REPORT("CMD: {}", command); }
	virtual ~ConsoleEntry() { Console::Unregister(name, this); }
};

// An auto registering Console Variable
template<typename T>
struct ConsoleVariable : public ConsoleEntry {
	using ValueType = T;
	T value;

	ConsoleVariable(const char* name, T defValue = {}, const char* inTooltip = "")
		: ConsoleEntry(name, inTooltip)
		, value(defValue)
	{
	}

	void Execute(std::string_view command) override
	{
		auto vec = str::Split(command);
		if (vec.size() <= 1) {
			LOG_REPORT("{}: {}", name, value);
			return;
		}
		value = conv::FromStrView<T>(vec[1]);
		LOG_REPORT("Console set {}: {}", name, value);
	}

	T& Get() { return value; }

	explicit operator T&() { return value; }
	virtual ~ConsoleVariable() = default;
};

// Auto registers a console function with a lambda.
struct ConsoleFunction : public ConsoleEntry {


	std::function<void(std::string_view)> function;

	// Func param can be left empty to be set later
	// Lambda params of func is a std::string_view of the console command (including the command itself)
	// Recommended to use with str::Split (StringAux) and conv::FromStrView. Automated templated variants to be provided
	// at a future update.
	ConsoleFunction(const char* name, std::function<void(std::string_view)> func = {}, const char* inTooltip = "")
		: ConsoleEntry(name, inTooltip)
		, function(func)
	{
	}

	void Execute(std::string_view command) override
	{
		if (function) {
			function(command);
			return;
		}
		LOG_REPORT("Console function: {} is registered but no function has been set.");
	}

	virtual ~ConsoleFunction() = default;
};