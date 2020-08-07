#pragma once


namespace ed {
// Generic struct for an editor context-less "operation". Includes code, keybinds (soon) and other info
struct Operation {
	std::string name;
	const char* icon = "";

	std::function<void()> invoke;
};


} // namespace ed
