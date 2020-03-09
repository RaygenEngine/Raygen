#include "Test.h"


#include "engine/console/Console.h"
#include "engine/console/ConsoleVariable.h"
#include "core/MathUtl.h"
#include <string_view>

ConsoleVariable<int32> g_variable("_t_var", 42);

TEST("Console Basic Variable")
{
	using namespace std::literals;
	REQ(g_variable.Get() == 42);

	Console::Execute("_t_var 52"sv);

	REQ(g_variable.Get() == 52);
}

ConsoleVariable<float> g_float("_t_floatv", 0.1f);
TEST("Console Float Variable")
{
	using namespace std::literals;
	REQ(math::equals(g_float.Get(), 0.1f));
	Console::Execute("_t_floatv 3.125e7"sv);
	REQ(math::equals(g_float.Get(), 3.125e7f));
}

TEST("Console Runtime registration")
{
	// Super dangerous local variable capture in global lambda. DON'T DO THIS IN REAL CODE
	using namespace std::literals;

	int32 var = -1;
	{
		ConsoleFunction<> g_func(
			"_t_callable", [&]() { var = 8; }); // This will get unregistered at the end of this scope

		REQ(var == -1);
		Console::Execute("_t_callable"sv);
		REQ(var == 8);
	}
	var = 1;
	Console::Execute("_t_callable"sv); // Should fail to find a function
	REQ(var == 1);
}

TEST("Console Automatic params")
{
	using namespace std::literals;

	int32 var = -1;
	ConsoleFunction<int32, float> g_func("_add", [&](auto x, auto y) {
		var = x + static_cast<int32>(y);
	}); // This will get unregistered at the end of this scope

	REQ(var == -1);
	Console::Execute("_add 1 2"sv);
	REQ(var == 3);
}

TEST("Console Var Func")
{
	using namespace std::literals;

	bool didUpdate = false;

	ConsoleVarFunc<int32> var(
		"_test", [&]() { didUpdate = true; }, -1);

	REQ(var.Get() == -1);

	Console::Execute("_test"sv);

	REQ(didUpdate == false); // Wont update on access

	Console::Execute("_test 2"sv);

	REQ(var.Get() == 2);    // Value properly set
	REQ(didUpdate == true); // function did run
}
