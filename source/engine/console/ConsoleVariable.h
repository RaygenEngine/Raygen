#pragma once
#include "core/StringConversions.h"
#include "core/StringUtl.h"
#include "engine/console/Console.h"
#include "engine/Logger.h"
#include "reflection/TypeId.h"
#include "reflection/ReflEnum.h"

#include <functional>
#include <string>

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


	[[nodiscard]] virtual std::string GetDescriptionLine() const
	{
		if (tooltip[0] == '\0') {
			return fmt::format("{}", name);
		}
		return fmt::format("{:<30} - {}", name, tooltip);
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

	[[nodiscard]] std::string GetDescriptionLine() const override
	{
		auto str = fmt::format("{}: [{}]", name, value);
		if constexpr (std::is_enum_v<T>) {
			MetaEnumInst enumInst = GenMetaEnum(value);
			str = fmt::format("{}: [{} ({})]", name, enumInst.GetValueStr(), value);
		}
		if (tooltip[0] == '\0') {
			return str;
		}
		return fmt::format("{:<30} - {}", str, tooltip);
	}

	void Execute(std::string_view command) override { TryUpdateValue(command); }

	[[nodiscard]] T& Get() { return value; }

	[[nodiscard]] operator T&() { return value; }

	virtual ~ConsoleVariable() = default;

protected:
	bool TryUpdateValue(std::string_view command)
	{
		if constexpr (std::is_enum_v<T>) {
			MetaEnumInst enumInst = GenMetaEnum(value);

			auto vec = str::split(command);
			if (vec.size() <= 1) {
				std::stringstream possibleValues;
				for (auto& val : enumInst.GetEnum().GetValues()) {
					possibleValues << "\t" << val.first << ": " << val.second << "\n";
				}

				LOG_REPORT("{}: {} ({})\n{}", name, enumInst.GetValueStr(), value, possibleValues.str());
				return false;
			}
			if (str::startsWithNum(vec[1])) {
				if (!enumInst.SetValue(str::fromStrView<std::underlying_type_t<T>>(vec[1]))) {
					LOG_REPORT("Out of bounds value for enum {}.", mti::GetName<T>());
					return false;
				}
			}
			else {
				if (!enumInst.SetValueByStr(std::string(vec[1]))) {
					LOG_REPORT("Unknown string value for enum {}.", mti::GetName<T>());
					return false;
				}
			}

			LOG_REPORT("Console set {}: {} ({})", name, enumInst.GetValueStr(), value);
		}
		else {
			auto vec = str::split(command);
			if (vec.size() <= 1) {
				LOG_REPORT("{}: {}", name, value);
				return false;
			}
			value = str::fromStrView<T>(vec[1]);
			LOG_REPORT("Console set {}: {}", name, value);
		}

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
	// Recommended to use with str::split (StringUtl) and str::fromStrView. Automated templated variants to be provided
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
// Automatically parses console using str::fromStrView (or operator<<)
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

		auto vec = str::split(command);
		if (vec.size() - 1 != argCount) {
			LOG_REPORT("Incorrect number of arguments: {}", GetDescriptionLine());
			return;
		}

		std::tuple<Args...> args;


		std::apply(
			[&](auto&&... elem) {
				int currentArg = 1;
				((void)(elem = str::fromStrView<std::remove_reference_t<decltype(elem)>>(vec[currentArg++])), ...);
			},
			args);

		std::apply(function, args);
	}

	[[nodiscard]] std::string GetDescriptionLine() const override
	{
		if constexpr (sizeof...(Args) > 0) {
			std::stringstream args;

			((args << mti::GetName<Args>() << ", "), ...);
			if (tooltip[0] == '\0') {
				return fmt::format("{} [{}]", name, args.str());
			}
			return fmt::format("{:<19} [{:<9}] - {}", name, args.str(), tooltip);
		}

		return ConsoleEntry::GetDescriptionLine();
	}


	virtual ~ConsoleFunction() = default;
};
