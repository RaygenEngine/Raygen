#pragma once
#include "editor/windows/EdWindow.h"
#include "system/profiler/ProfilerSetup.h"
#include "asset/AssetManager.h"

namespace ed {
// The "programmer" friendly version of an asset view.
// Does not use folders / other organization methos and just shows a good old asset list of ALL the asset entries.
class AssetListWindow : public UniqueWindow {
public:
	AssetListWindow(std::string_view name);

	virtual void ImguiDraw();
	virtual ~AssetListWindow() = default;

private:
	void DrawEntry(PodEntry& entry);
	bool m_isVisible{ true };
	bool m_showUnsaved{ true };
	bool m_showSaved{ true };
	bool m_showTransient{ false };
};

} // namespace ed
