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
class ViewportWindow : public UniqueWindow {
public:
	ViewportWindow(const std::string& name)
		: UniqueWindow(name)
	{
	}

	void OnDraw(const char* title, bool* keepOpen) override
	{

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse;
		//| ImGuiWindowFlags_NoMouseInputs;
		auto copy = g_ViewportCoordinates;

		if (ImGui::Begin(title, nullptr, flags)) {
			auto windowRelative = glm::uvec2(ImGui::GetWindowPos().x - ImGui::GetWindowViewport()->Pos.x,
				ImGui::GetWindowPos().y - ImGui::GetWindowViewport()->Pos.y);

			g_ViewportCoordinates.position = { windowRelative.x, windowRelative.y };
			g_ViewportCoordinates.size = { ImGui::GetWindowSize().x, ImGui::GetWindowSize().y };

			if (copy != g_ViewportCoordinates) {
				Event::OnViewportUpdated.Broadcast();
			}

			auto str = fmt::format("Pos: {}, {} Size: {} {}", windowRelative.x, windowRelative.y,
				g_ViewportCoordinates.size.x, g_ViewportCoordinates.size.y);


			ImGui::Text(str.c_str());
		}

		ImGui::End();
	}
};

void RegisterWindows(ed::ComponentWindows& windowsComponent)
{
	windowsComponent.AddWindowEntry<AssetsWindow>("Assets");
	windowsComponent.AddWindowEntry<ViewportWindow>("Viewport");

	windowsComponent.AddWindowEntry<AboutWindow>("About");
	windowsComponent.AddWindowEntry<HelpWindow>("Help");
	windowsComponent.AddWindowEntry<ConsoleWindow>("Console");

	windowsComponent.AddWindowEntry<ProfilerWindow>("Profiler");
}
} // namespace ed
