#pragma once
#include <array>

namespace ProfilerSetup {
// All profiler modules
enum Module : size_t
{
	Core,
	System,
	Editor,
	Asset,
	Platform,
	Renderer,
	World,
	Game,
};

//
// Profiler Setup, updating this will trigger recompile of all translation units that profile something
//
constexpr bool c_startsEnabled = true;
constexpr std::array Enabled = {
	//	Core,
	System, Editor,
	//	Asset,
	//	Platform,
	Renderer, World,
	//	Game,
};

//
//
//
constexpr bool IsEnabled(Module m)
{
	for (const auto& v : Enabled) {
		if (v == m) {
			return true;
		}
	}
	return false;
}

}; // namespace ProfilerSetup
