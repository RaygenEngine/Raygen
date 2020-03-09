#include "pch.h"

#include "editor/windows/WindowsRegistry.h"
#include "editor/EdComponentWindows.h"

#include "engine/Events.h"

#include "editor/windows/general/EdMiscWindow.h"
#include "editor/windows/general/EdConsoleWindow.h"
#include "editor/windows/general/EdProfilerWindow.h"
#include "editor/windows/general/EdAssetsWindow.h"
#include "editor/windows/general/EdAssetListWindow.h"


#include "glfw/glfw3.h"
#include "imgui/imgui.h"


namespace ed {
void RegisterWindows(ed::ComponentWindows& windowsComponent)
{
	windowsComponent.AddWindowEntry<AssetsWindow>("Asset Browser");
	windowsComponent.AddWindowEntry<AssetListWindow>("Asset List");

	windowsComponent.AddWindowEntry<ConsoleWindow>("Console");

	windowsComponent.AddWindowEntry<ProfilerWindow>("Profiler");

	windowsComponent.AddWindowEntry<AboutWindow>("About");
	windowsComponent.AddWindowEntry<HelpWindow>("Help");
	windowsComponent.AddWindowEntry<ImGuiDemoWindow>("ImGui Demo");
}
} // namespace ed
