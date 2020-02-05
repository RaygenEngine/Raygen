#include "pch/pch.h"
#include "editor/windows/general/EdMiscWindow.h"
#include "editor/DataStrings.h"

#include "editor/imgui/ImguiUtil.h"

#include <imgui/imgui.h>
namespace ed {
void HelpWindow::OnDraw(const char* title, bool* open)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
	ImGui::SetNextWindowPos(ImVec2(650.f, 100.f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(0, 720.f), ImGuiCond_Always);

	if (ImGui::Begin(title, open)) {
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 33.0f);
		ImGui::Text(txt_help);
		ImGui::Text("");
	}

	ImGui::End();
	ImGui::PopStyleVar();
}

void AboutWindow::ImguiDraw()
{
	ImGui::Text("Raygen: v0.1");
	ImExt::HSpace(220.f);
	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 33.0f);
	ImGui::Text(txt_about);
	ImGui::Text("");
}


} // namespace ed
