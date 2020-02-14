#include "pch/pch.h"

#include "editor/windows/WindowsRegistry.h"
#include "editor/EdComponentWindows.h"

#include "system/EngineEvents.h"

#include "editor/windows/general/EdMiscWindow.h"
#include "editor/windows/general/EdConsoleWindow.h"
#include "editor/windows/general/EdProfilerWindow.h"
#include "editor/windows/general/EdAssetsWindow.h"

#include "glfw/glfw3.h"
#include "imgui/imgui.h"


namespace ed {
void RegisterWindows(ed::ComponentWindows& windowsComponent)
{
	windowsComponent.AddWindowEntry<AssetsWindow>("Assets");

	windowsComponent.AddWindowEntry<ConsoleWindow>("Console");

	windowsComponent.AddWindowEntry<ProfilerWindow>("Profiler");

	windowsComponent.AddWindowEntry<AboutWindow>("About");
	windowsComponent.AddWindowEntry<HelpWindow>("Help");
	windowsComponent.AddWindowEntry<ImGuiDemoWindow>("ImGui Demo");
}
} // namespace ed
