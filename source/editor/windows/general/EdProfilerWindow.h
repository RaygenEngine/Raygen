#pragma once
#include "editor/windows/EdWindow.h"

#include "engine/profiler/ProfilerSetup.h"

namespace ed {
class ProfilerWindow : public UniqueWindow {
public:
	ProfilerWindow(std::string_view name);

protected:
	// CHECK: not needed as static, but we want compiletime size, (can be done with template)
	static inline std::array<bool, 32> visibleCategories{};

	void DrawCategoryContents(ProfilerSetup::Module category);

	int32 m_currentExportFrame{ 0 }; // when 0 we are not exporting burst of frames

public:
	virtual void ImguiDraw();
	virtual ~ProfilerWindow() = default;
};

} // namespace ed
