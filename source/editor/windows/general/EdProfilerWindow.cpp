#include "pch.h"
#include "editor/windows/general/EdProfilerWindow.h"
#include "engine/profiler/ProfileScope.h"
#include "editor/misc/NativeFileBrowser.h"

#include "reflection/ReflEnum.h"
#include <spdlog/fmt/fmt.h>
#include "editor/imgui/ImEd.h"

namespace ed {

constexpr size_t c_catNum = magic_enum::enum_count<ProfilerSetup::Module>();
constexpr auto c_catNames = magic_enum::enum_names<ProfilerSetup::Module>();
[[nodiscard]] constexpr std::array<std::string_view, c_catNum> MakeCategories()
{
	std::array<std::string_view, c_catNum> r{};
	for (auto& v : ProfilerSetup::Enabled) {
		r[v] = c_catNames[v];
	}
	return r;
}
constexpr auto c_categories = MakeCategories();

ProfilerWindow::ProfilerWindow(std::string_view name)
	: UniqueWindow(name)
{
	static_assert(visibleCategories.size() > c_catNum,
		"Increase visibile categories array to match all possible profiler module entries.");
}


void ProfilerWindow::DrawCategoryContents(ProfilerSetup::Module category)
{
	auto* vec = Profiler.GetModule(category);
	if (!vec) {
		return;
	}

	auto frametime = Profiler.GetLastFrameTime();

	if (frametime.count() == 0) {
		return;
	}


	auto& entries = *vec;

	for (auto& entry : entries) {
		float perc = static_cast<float>(entry->prevSumDuration.count()) / frametime.count();
		auto totWidth = ImGui::GetContentRegionAvail().x;

		ImGui::ProgressBar(perc, ImVec2(totWidth / 4.f, 0));
		ImGui::SameLine();

		std::string loc = fmt::format("{:.{}} [{}]", entry->frontFacingName,
			static_cast<int32>((totWidth * 1.2) / std::max(ImGui::GetFontSize(), 1.f)), entry->line);
		ImEd::SetNextItemPerc(0.3f);
		ImGui::Text(loc.c_str());
		ImGui::SameLine(totWidth * 0.85f);

		std::string hits;
		if (entry->prevHits > 0) {
			const float c_PrecisionToMicros = 1.f / (1000.f);
			auto count = entry->prevSumDuration.count();
			float perHit = (static_cast<float>(count) / entry->prevHits) * c_PrecisionToMicros;

			// CHECK: C++20 use chrono operator<<
			// CHECK: assumes precision
			float micros = c_PrecisionToMicros * count;

			if (micros < 1000.f) {
				hits = fmt::format("{:04.2f} us", micros);

				ImGui::Text(hits.c_str());
				hits = fmt::format("{}", entry->prevHits);
				ImGui::SameLine(totWidth * 0.975f);
				ImGui::Text(hits.c_str());
				TEXT_TOOLTIP("Number of times hit last frame.\n\n{} Hits\n{:04.2f} us/hit", entry->prevHits, perHit);
			}
			else {

				hits = fmt::format("{:04.2f} ms", micros / 1000.f);
				ImGui::Text(hits.c_str());

				hits = fmt::format("{}", entry->prevHits);
				ImGui::SameLine(totWidth * 0.98f);
				ImGui::Text(hits.c_str());
				TEXT_TOOLTIP(
					"Number of times hit last frame.\n\n{} Hits\n{:04.2f} ms/hit", entry->prevHits, perHit / 1000.f);
			}
		}
	}
}


void ProfilerWindow::ImguiDraw()
{
	bool current = Profiler.m_isProfiling;

	if (m_currentExportFrame > 0) {
		ImGui::Text("Recording frames...");

		if (m_currentExportFrame == 1) {
			Profiler.ResetSession();
			Profiler.BeginProfiling();
		}
		else if (m_currentExportFrame == 11) {
			Profiler.EndProfiling();
		}
		else if (m_currentExportFrame == 12) {
			if (auto file = ed::NativeFileBrowser::SaveFile({ "json" })) {
				Profiler.ExportSessionToJson(*file);
			}
			m_currentExportFrame = 0;
			Profiler.ResetSession();
			return;
		}
		m_currentExportFrame++;
		return;
	}

	if (current) {
		if (ImEd::Button("Pause Profiling")) {
			Profiler.EndProfiling();
		}
	}
	else {
		if (ImEd::Button("Start Profiling")) {
			Profiler.BeginProfiling();
		}
	}
	ImGui::SameLine();
	if (ImEd::Button("Export Session")) {
		if (auto file = ed::NativeFileBrowser::SaveFile({ "json" })) {
			Profiler.ExportSessionToJson(*file);
		}
	}
	ImGui::SameLine();
	if (ImEd::Button("Reset Session")) {
		Profiler.ResetSession();
	}

	ImGui::SameLine();
	if (ImEd::Button("Export Frames")) {
		m_currentExportFrame = 1;
		Profiler.EndProfiling();
	}


	ImGuiStyle& style = ImGui::GetStyle();
	float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
	constexpr float c_colWidth = 120.f;
	bool lastHasSameLine = false;

	for (int i = 0; i < c_catNum; ++i) {
		if (c_categories[i].size() == 0) {
			continue;
		}
		ImGui::PushID(i);

		auto str = std::string(c_categories[i]);
		auto beginCursorPos = ImGui::GetCursorPosX();
		ImGui::Checkbox(str.c_str(), &visibleCategories[i]);
		lastHasSameLine = false;
		float last_button_x2 = ImGui::GetItemRectMin().x + c_colWidth;
		float next_button_x2 = last_button_x2 + c_colWidth; // Expected position if next button was on same line

		if (next_button_x2 < window_visible_x2) {
			ImGui::SameLine();
			ImGui::SetCursorPosX(beginCursorPos + c_colWidth);
			lastHasSameLine = true;
		}
		ImGui::PopID();
	}
	if (lastHasSameLine) {
		ImGui::NewLine();
	}
	ImGui::Separator();

	for (int32 i = 0; i < c_catNum; i++) {
		if (visibleCategories[i]) {
			DrawCategoryContents(static_cast<ProfilerSetup::Module>(i));
			ImGui::Separator();
		}
	}
}
} // namespace ed
