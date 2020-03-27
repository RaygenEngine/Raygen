#pragma once

#include <array>

namespace ProfilerSetup {
// All profiler modules
enum Module : size_t
{
	Core,
	Engine,
	EditorObject_,
	Asset,
	Platform,
	Renderer,
	World,
	Game,
};

//
// Profiler Setup, updating this will trigger recompile of all translation units that profile something
//


constexpr bool c_startsEnabled = false;

// Extended is more expensive but has extra data
constexpr bool c_isDefaultScopeExtended = true;
constexpr bool c_shouldOverrideAllWithDefault = false;


// Enabled Modules
constexpr std::array Enabled = {
	//	Core,
	Engine, EditorObject_,
	//	Asset,
	//	Platform,
	Renderer, World,
	//	Game,
};

// End of config section
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
