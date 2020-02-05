#include "pch/pch.h"
#include "editor/windows/general/EdConsoleWindow.h"
#include "system/console/Console.h"


#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

namespace ed {
void ConsoleWindow::ImguiDraw()
{
	if (ImGui::InputText("###ConsoleInp", &m_input, ImGuiInputTextFlags_EnterReturnsTrue)) {
		Console::Execute(m_input);
		m_input = "";
	}
	ImGui::Separator();
}
} // namespace ed
