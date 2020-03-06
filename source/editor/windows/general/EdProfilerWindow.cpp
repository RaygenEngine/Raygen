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


void ProfilerWindow::ShowCategoryCheckbox(ProfilerSetup::Module category) {}

void ProfilerWindow::DrawCategoryContents(ProfilerSetup::Module category)
{
	auto* vec = Profiler::GetModule(category);
	if (!vec) {
		return;
	}

	auto frametime = Profiler::GetLastFrameTime();

	if (frametime.count() == 0) {
		return;
	}


	auto& entries = *vec;

	for (auto& entry : entries) {


		float perc = static_cast<float>(entry->sumDuration.count()) / frametime.count();


		auto totWidth = ImGui::GetContentRegionAvail().x;


		ImGui::ProgressBar(perc, ImVec2(totWidth / 4.f, 0));
		ImGui::SameLine();

		std::string loc = fmt::format("{} {}", entry->frontFacingName, entry->line);

		ImEd::SetNextItemPerc(0.3f);
		ImGui::Text(loc.c_str());
		ImGui::SameLine(totWidth * 0.70f);


		std::string hits;
		if (entry->hits > 0) {
			const float c_PrecisionToMicros = 1.f / (1000.f);
			auto count = entry->sumDuration.count();
			float perHit = (static_cast<float>(count) / entry->hits) * c_PrecisionToMicros;

			// CHECK: C++20 use chrono operator<<
			// CHECK: assumes precision
			float micros = c_PrecisionToMicros * count;

			if (micros < 1000.f) {
				hits = fmt::format("{:04.2f} us", micros);
				ImGui::Text(hits.c_str());

				hits = fmt::format("{} Hits | {:04.2f} us avg.", entry->hits, perHit);
				ImGui::SameLine(totWidth * 0.8f);
				ImGui::Text(hits.c_str());
			}
			else {

				hits = fmt::format("{:04.2f} ms", micros / 1000.f);
				ImGui::Text(hits.c_str());

				hits = fmt::format("{} Hits | {:04.2f} ms avg.", entry->hits, perHit / 1000.f);
				ImGui::SameLine(totWidth * 0.8f);
				ImGui::Text(hits.c_str());
			}
		}
		else {
			hits = fmt::format("0 | 0 Hits");
			ImGui::Text(hits.c_str());
		}
	}
}


void ProfilerWindow::ImguiDraw()
{
	bool current = Profiler::s_isProfiling;

	if (current) {
		if (ImEd::Button("Pause Profiling")) {
			Profiler::EndProfiling();
		}
	}
	else {
		if (ImEd::Button("Start Profiling")) {
			Profiler::BeginProfiling();
		}
	}
	ImGui::SameLine();
	if (ImEd::Button("Export Session")) {
		if (auto file = ed::NativeFileBrowser::SaveFile({ "json" })) {
			Profiler::ExportSessionToJson(*file);
		}
	}

	ImGui::SameLine();
	if (ImEd::Button("Reset Session")) {
		Profiler::ResetSession();
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
