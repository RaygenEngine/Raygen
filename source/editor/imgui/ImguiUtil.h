#pragma once
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#define TEXT_TOOLTIP(...)                                                                                              \
	if (ImGui::IsItemHovered()) {                                                                                      \
		ImUtil::TextTooltipUtil(fmt::format(__VA_ARGS__));                                                             \
	}

namespace ImUtil {

inline void TextTooltipUtil(const std::string& Tooltip, float tooltipTextScale = 1.f)
{
	ImGui::BeginTooltip();
	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
	ImGui::SetWindowFontScale(tooltipTextScale);
	ImGui::TextUnformatted(Tooltip.c_str());
	ImGui::PopTextWrapPos();
	ImGui::EndTooltip();
}

[[deprecated("Use glm::value_ptr")]] inline float* FromVec3(glm::vec3& vec3)
{
	return glm::value_ptr(vec3);
}

[[deprecated("Use glm::value_ptr")]] inline float* FromVec4(glm::vec4& vec4)
{
	return glm::value_ptr(vec4);
}

} // namespace ImUtil
