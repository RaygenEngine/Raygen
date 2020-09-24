#pragma once
#include "editor/windows/EdWindow.h"

namespace ed {
class AboutWindow : public ed::UniqueWindow {
public:
	AboutWindow(std::string_view name)
		: ed::UniqueWindow(name)
	{
	}

	virtual void ImguiDraw();
	virtual ~AboutWindow() = default;
};

class HelpWindow : public ed::UniqueWindow {
public:
	HelpWindow(std::string_view name)
		: ed::UniqueWindow(name)
	{
	}

	virtual void OnDraw(const char* title, bool* open);


	virtual ~HelpWindow() = default;
};
class ImGuiDemoWindow : public ed::UniqueWindow {
public:
	inline static const char* Category = "Debug";

	ImGuiDemoWindow(std::string_view name)
		: ed::UniqueWindow(name)
	{
	}

	virtual void OnDraw(const char* title, bool* open);


	virtual ~ImGuiDemoWindow() = default;
};

class PodEntryEditorWindow : public UniqueWindow {
public:
	PodEntryEditorWindow(std::string_view name)
		: ed::UniqueWindow(name)
	{
	}
	PodEntry* entry = {};

	void ImguiDraw() override;
};

} // namespace ed
