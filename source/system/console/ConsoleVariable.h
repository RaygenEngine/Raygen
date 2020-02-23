#pragma once
#include "core/StringAux.h"
#include "core/StringConversions.h"
#include "system/console/Console.h"
#include "system/Logger.h"
#include <string_view>
#include <functional>

// TODO: Genericly support strings in quotes as parameters

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

	void Execute(std::string_view command) override { TryUpdateValue(command); }

	T& Get() { return value; }

	explicit operator T&() { return value; }
	virtual ~ConsoleVariable() = default;

protected:
	bool TryUpdateValue(std::string_view command)
	{
		auto vec = str::Split(command);
		if (vec.size() <= 1) {
			LOG_REPORT("{}: {}", name, value);
			return false;
		}
		value = conv::FromStrView<T>(vec[1]);
		LOG_REPORT("Console set {}: {}", name, value);
		return true;
	}
};


template<typename T>
struct ConsoleVarFunc : public ConsoleVariable<T> {
	std::function<void()> updateFunc;

	ConsoleVarFunc(const char* name, std::function<void()>&& function, T defValue = {}, const char* inTooltip = "")
		: ConsoleVariable(name, defValue, inTooltip)
		, updateFunc(function)
	{
	}

	void Execute(std::string_view command) override
	{
		if (TryUpdateValue(command)) {
			updateFunc();
		}
	}
};

// Auto registers a console function with a lambda.
struct ConsoleFunctionGeneric : public ConsoleEntry {


	std::function<void(std::string_view)> function;

	// Func param can be left empty to be set later
	// Lambda params of func is a std::string_view of the console command (including the command itself)
	// Recommended to use with str::Split (StringAux) and conv::FromStrView. Automated templated variants to be provided
	// at a future update.
	ConsoleFunctionGeneric(
		const char* name, std::function<void(std::string_view)> func = {}, const char* inTooltip = "")
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
		LOG_REPORT("Console function: {} is registered but no function has been set.", name);
	}

	virtual ~ConsoleFunctionGeneric() = default;
};

// Templated console function.
// Declared as: ConsoleFunction<int32, int32> myConsoleFunction{"callme"};
// Automatically parses console using conv::FromStrView (or operator<<)
template<typename... Args>
struct ConsoleFunction : public ConsoleEntry {

	std::function<void(Args...)> function;

	static constexpr auto argCount = sizeof...(Args);

	ConsoleFunction(const char* name, std::function<void(Args...)> func = {}, const char* inTooltip = "")
		: ConsoleEntry(name, inTooltip)
		, function(func)
	{
	}

	void Execute(std::string_view command) override
	{
		if (!function) {
			LOG_REPORT("Console function: {} is registered but no function has been set.", name);
			return;
		}

		auto vec = str::Split(command);
		if (vec.size() - 1 != argCount) {
			LOG_REPORT("Incorrect number of arguments for console command: {}. Expected: {}", name, argCount);
			return;
		}

		std::tuple<Args...> args;


		std::apply(
			[&](auto&&... elem) {
				int currentArg = 1;
				((void)(elem = conv::FromStrView<std::remove_reference_t<decltype(elem)>>(vec[currentArg++])), ...);
			},
			args);

		std::apply(function, args);
	}


	virtual ~ConsoleFunction() = default;
};
