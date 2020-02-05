#include "pch/pch.h"

#include "editor/windows/WindowsRegistry.h"
#include "editor/EdComponentWindows.h"

#include "editor/windows/general/EdMiscWindow.h"
#include "editor/windows/general/EdConsoleWindow.h"
#include "editor/windows/general/EdProfilerWindow.h"


namespace ed {
void RegisterWindows(ed::ComponentWindows& windowsComponent)
{
	windowsComponent.AddWindowEntry<AboutWindow>("About");
	windowsComponent.AddWindowEntry<HelpWindow>("Help");
	windowsComponent.AddWindowEntry<ConsoleWindow>("Console");
	windowsComponent.AddWindowEntry<ProfilerWindow>("Profiler");
}
} // namespace ed
