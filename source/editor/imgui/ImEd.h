#pragma once
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

// ImGui wrapper for calls that use different styles. (bigger buttons etc)
// header file to allow inlining
namespace ImEd {
bool Button(const char* label, const ImVec2& size = ImVec2(0.f, 0.f))
{
	bool result;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
	result = ImGui::Button(label, size);
	ImGui::PopStyleVar();
	return result;
}

void SetNextItemPerc(float perc)
{
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * perc);
}

} // namespace ImEd
