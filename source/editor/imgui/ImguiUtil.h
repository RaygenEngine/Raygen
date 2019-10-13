#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>
#include <imgui_ext/imfilebrowser.h>
#include <imgui/imgui_internal.h>

#define TEXT_TOOLTIP(...)                                                                                              \
	if (ImGui::IsItemHovered()) {                                                                                      \
		ImUtil::TextTooltipUtil(fmt::format(__VA_ARGS__));                                                             \
	}

namespace ImUtil {

inline void TextTooltipUtil(const std::string& Tooltip)
{
	ImGui::BeginTooltip();
	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 45.0f);
	ImGui::TextUnformatted(Tooltip.c_str());
	ImGui::PopTextWrapPos();
	ImGui::EndTooltip();
}

inline float* FromVec3(glm::vec3& vec3)
{
	return reinterpret_cast<float*>(&vec3);
}

inline float* FromVec4(glm::vec4& vec4)
{
	return reinterpret_cast<float*>(&vec4);
}

} // namespace ImUtil
