#pragma once
#include "editor/windows/EdWindow.h"

#include "system/profiler/ProfilerSetup.h"

namespace ed {
class ProfilerWindow : public UniqueWindow {
public:
	ProfilerWindow(std::string_view name);

protected:
	// TODO: not needed as static, but we want compiletime size, (can be done with template)
	static inline std::array<bool, 32> visibleCategories{};
	void ShowCategoryCheckbox(ProfilerSetup::Module category);

	void DrawCategoryContents(ProfilerSetup::Module category);

public:
	virtual void ImguiDraw();
	virtual ~ProfilerWindow() = default;
};

} // namespace ed
