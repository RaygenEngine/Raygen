#include "Test.h"


#include "system/console/Console.h"
#include "system/console/ConsoleVariable.h"
#include "core/MathAux.h"
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
	REQ(math::EpsilonEquals(g_float.Get(), 0.1f));
	Console::Execute("_t_floatv 3.125e7"sv);
	REQ(math::EpsilonEquals(g_float.Get(), 3.125e7f));
}

TEST("Console Runtime registration")
{
	// Super dangerous local variable capture in global lambda. DON'T DO THIS IN REAL CODE
	using namespace std::literals;

	int32 var = -1;
	{
		ConsoleFunction g_func(
			"_t_callable", [&](auto) { var = 8; }); // This will get unregistered at the end of this scope

		REQ(var == -1);
		Console::Execute("_t_callable"sv);
		REQ(var == 8);
	}
	var = 1;
	Console::Execute("_t_callable"sv); // Should fail to find a function
	REQ(var == 1);
}
