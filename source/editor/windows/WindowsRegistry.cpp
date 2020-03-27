#include "pch.h"
#include "WindowsRegistry.h"

#include "editor/EdComponentWindows.h"
#include "editor/windows/general/EdAssetListWindow.h"
#include "editor/windows/general/EdAssetsWindow.h"
#include "editor/windows/general/EdConsoleWindow.h"
#include "editor/windows/general/EdMiscWindow.h"
#include "editor/windows/general/EdOutlinerWindow.h"
#include "editor/windows/general/EdProfilerWindow.h"
#include "editor/windows/general/EdPropertyEditorWindow.h"
#include "engine/Events.h"

namespace ed {
void RegisterWindows(ed::ComponentWindows& windowsComponent)
{
	windowsComponent.AddWindowEntry<OutlinerWindow>("Outliner");
	windowsComponent.AddWindowEntry<PropertyEditorWindow>("Property Editor");

	windowsComponent.AddWindowEntry<AssetsWindow>("Asset Browser");
	windowsComponent.AddWindowEntry<AssetListWindow>("Asset List");

	windowsComponent.AddWindowEntry<ConsoleWindow>("Console");

	windowsComponent.AddWindowEntry<GBufferDebugWindow>("GBuffer Debugger");


	windowsComponent.AddWindowEntry<ProfilerWindow>("Profiler");

	windowsComponent.AddWindowEntry<AboutWindow>("About");
	windowsComponent.AddWindowEntry<HelpWindow>("Help");
	windowsComponent.AddWindowEntry<ImGuiDemoWindow>("ImGui Demo");
}
} // namespace ed
