#pragma once
#include "editor/windows/EdWindow.h"

namespace ed {
class ConsoleWindow : public UniqueWindow {
	std::string m_input;

public:
	ConsoleWindow(std::string_view name)
		: UniqueWindow(name)
	{
	}

	virtual void OnDraw(const char* title, bool* keepOpen);

	virtual ~ConsoleWindow() = default;
};

} // namespace ed
