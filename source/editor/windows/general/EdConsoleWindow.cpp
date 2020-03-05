#include "pch.h"
#include "editor/windows/general/EdConsoleWindow.h"
#include "system/console/Console.h"

#include "editor/imgui/ImguiImpl.h"
#include "editor/imgui/ImEd.h"

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>


namespace {
// CHECK: fix log levels here when/if logger gets redone


struct ImguiLogState {
	ImGuiTextBuffer Buf;
	ImGuiTextFilter filter;
	ImVector<int> LineOffsets; // Index to lines offset. We maintain this with AddLog() calls, allowing us to
							   // have a random access on lines
	bool AutoScroll;           // Keep scrolling if already at the bottom

	std::string consoleStr{};
	bool shouldRecaptureKeyboard{ false };

	ImguiLogState()
	{
		AutoScroll = true;
		Clear();
	}

	void Clear()
	{
		Buf.clear();
		LineOffsets.clear();
		LineOffsets.push_back(0);
	}

	void AddLog(const std::string& str)
	{
		int32 oldSize = Buf.size();
		Buf.append("\n");
		Buf.append(str.c_str());
		for (int newSize = Buf.size(); oldSize < newSize; oldSize++) {
			if (Buf[oldSize] == '\n') {
				LineOffsets.push_back(oldSize + 1);
			}
		}
	}

	ImVec4 c_warnColor = { 0.9f, 0.66f, 0.38f, 1.f };
	ImVec4 c_errColor = { 1.f, 0.f, 0.f, 1.f };

	LogLevel GetLineLogLevel(const char* begin) const
	{
		// HACK: access 15 directly (hard coded) which is the letter of the log "type" (
		// corresponds to formatting in logger.cpp for the editor log sink
		constexpr int32 typeLetterIndex = 15;
		const char type = begin[typeLetterIndex];
		switch (type) {
			case 'T': return LogLevel::Trace;
			case 'D': return LogLevel::Debug;
			case 'I': return LogLevel::Info;
			case 'W': return LogLevel::Warn;
			case 'E': return LogLevel::Error;
			case 'C': return LogLevel::Critical;
		}
		return LogLevel::Off;
	}

	void DrawTxtLine(const char* line_start, const char* line_end)
	{
		bool colored{ false };
		auto level = GetLineLogLevel(line_start);
		switch (level) {
			case LogLevel::Warn: ImGui::PushStyleColor(ImGuiCol_Text, c_warnColor); break;
			case LogLevel::Error: ImGui::PushStyleColor(ImGuiCol_Text, c_errColor); break;
			default: {
				ImGui::TextUnformatted(line_start, line_end);
				return;
			}
		}
		ImGui::TextUnformatted(line_start, line_end);
		ImGui::PopStyleColor();
	}

	void Draw(const char* title, bool* p_open = NULL)
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Cmd: ");
		ImGui::SameLine();

		if (shouldRecaptureKeyboard) {
			shouldRecaptureKeyboard = false;
			ImGui::SetKeyboardFocusHere();
		}

		if (ImGui::InputText("##ConsoleInput", &consoleStr, ImGuiInputTextFlags_EnterReturnsTrue)) {
			Console::Execute(consoleStr);
			consoleStr = "";
			shouldRecaptureKeyboard = true;
		}

		ImEd::HSpace();
		ImGui::SameLine();
		ImGui::Text("Filter:");
		ImGui::SameLine();
		filter.Draw("##LogFilter");

		ImGui::Separator();
		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, -1.5f));
		ImGui::PushFont(ImguiImpl::s_CodeFont);

		const char* buf = Buf.begin();
		const char* buf_end = Buf.end();
		if (filter.IsActive()) {
			// In this example we don't use the clipper when Filter is enabled.
			// This is because we don't have a random access on the result on our filter.
			// A real application processing logs with ten of thousands of entries may want to store the result
			// of search/filter. especially if the filtering function is not trivial (e.g. reg-exp).
			for (int line_no = 0; line_no < LineOffsets.Size; line_no++) {
				const char* line_start = buf + LineOffsets[line_no];
				const char* line_end
					= (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;

				if (filter.PassFilter(line_start, line_end)) {
					DrawTxtLine(line_start, line_end);
				}
			}
		}
		else {

			// The simplest and easy way to display the entire buffer:
			//   ImGui::TextUnformatted(buf_begin, buf_end);
			// And it'll just work. TextUnformatted() has specialization for large blob of text and will
			// fast-forward to skip non-visible lines. Here we instead demonstrate using the clipper to only
			// process lines that are within the visible area. If you have tens of thousands of items and their
			// processing cost is non-negligible, coarse clipping them on your side is recommended. Using
			// ImGuiListClipper requires A) random access into your data, and B) items all being the  same
			// height, both of which we can handle since we an array pointing to the beginning of each line of
			// text. When using the filter (in the block of code above) we don't have random access into the
			// data to display anymore, which is why we don't use the clipper. Storing or skimming through the
			// search result would make it possible (and would be recommended if you want to search through tens
			// of thousands of entries)
			ImGuiListClipper clipper;
			clipper.Begin(LineOffsets.Size);
			while (clipper.Step()) {
				for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++) {
					const char* line_start = buf + LineOffsets[line_no];
					const char* line_end
						= (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;

					DrawTxtLine(line_start, line_end);
				}
			}
			clipper.End();
		}
		ImGui::PopFont();
		ImGui::PopStyleVar();

		if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
			ImGui::SetScrollHereY(1.0f);
		}


		ImGui::EndChild();
		if (ImGui::BeginPopupContextItem("ContextClick##ConsoleWindow")) {
			ImGui::Checkbox("Auto-scroll", &AutoScroll);
			if (ImGui::Button("Clear")) {
				Clear();
			}

			ImGui::EndPopup();
		}
	}
}; // namespace
} // namespace

namespace ed {

void ConsoleWindow::OnDraw(const char* title, bool* keepOpen)
{
	static ImguiLogState log;
	auto& ss = Log::s_editorLogStream;

	std::string line;
	while (std::getline(ss, line)) {
		log.AddLog(line.c_str());
	}
	ss.clear();

	ImGui::SetNextWindowPos(ImVec2(700.f, 300.f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin(title, keepOpen);

	log.Draw(title, keepOpen);
	ImGui::End();
}
} // namespace ed
