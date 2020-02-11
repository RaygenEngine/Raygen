#pragma once
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

// ImGui wrapper for calls that use different styles. (bigger buttons etc)
// header file to allow inlining
namespace ImEd {
inline bool Button(const char* label, const ImVec2& size = ImVec2(0.f, 0.f))
{
	bool result;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));
	result = ImGui::Button(label, size);
	ImGui::PopStyleVar();
	return result;
}

inline void SetNextItemPerc(float perc)
{
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * perc);
}

inline bool BeginMenuBar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.f, 7.f)); // On edit update imextras.h c_MenuPaddingY
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.f, 7.f));
	if (!ImGui::BeginMenuBar()) {
		ImGui::PopStyleVar(2);
		return false;
	}
	return true;
}

inline void EndMenuBar()
{
	ImGui::EndMenuBar();
	ImGui::PopStyleVar(2);
}

inline bool BeginMenu(const char* label, bool enabled = true)
{
	bool open = ImGui::BeginMenu(label, enabled);
	if (open) {
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 3.f));
		ImGui::Spacing();
		ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.f, 7.f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5.f, 7.f));
		return true;
	}
	return false;
}

inline void EndMenu()
{
	ImGui::PopStyleVar(2);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, -3.f));
	ImGui::Spacing();
	ImGui::PopStyleVar();
	ImGui::EndMenu();
}

inline void HSpace(float space = 6.f)
{
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(space, 0.f));
}


} // namespace ImEd
