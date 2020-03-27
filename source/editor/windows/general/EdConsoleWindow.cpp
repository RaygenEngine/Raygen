#include "pch.h"
#include "editor/windows/general/EdConsoleWindow.h"

#include "editor/imgui/ImEd.h"
#include "editor/imgui/ImguiImpl.h"
#include "engine/console/Console.h"

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>


namespace {
// CHECK: fix log levels here when/if logger gets redone

// Copied directly from ImGui Demo and mostly unmodified. As such the code uses mixed style, as well as malloc, free and
// other c-like code
struct ConsoleState {
	char inputBuf[256];

	ImGuiTextBuffer buffer;
	ImVector<int> lineOffsets;

	ImVector<char*> history;
	int historyPos; // -1: new line, 0..history.Size-1 browsing history.
	ImGuiTextFilter filter;
	bool autoScroll{ true };
	bool scrollToBottom{ false };
	LogLevel cachedLineLevel{ LogLevel::Off };

	ConsoleState()
	{
		ClearLog();
		memset(inputBuf, 0, sizeof(inputBuf));
		historyPos = -1;
		autoScroll = true;
		scrollToBottom = false;
		AddLog(" ");
	}
	~ConsoleState()
	{
		ClearLog();
		for (int i = 0; i < history.Size; i++) {
			free(history[i]);
		}
	}

	// Portable helpers
	static int Stricmp(const char* str1, const char* str2)
	{
		int d;
		while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) {
			str1++;
			str2++;
		}
		return d;
	}
	static int Strnicmp(const char* str1, const char* str2, int n)
	{
		int d = 0;
		while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) {
			str1++;
			str2++;
			n--;
		}
		return d;
	}
	static char* Strdup(const char* str)
	{
		size_t len = strlen(str) + 1;
		void* buf = malloc(len);
		IM_ASSERT(buf);
		return (char*)memcpy(buf, (const void*)str, len);
	}
	static void Strtrim(char* str)
	{
		char* str_end = str + strlen(str);
		while (str_end > str && str_end[-1] == ' ')
			str_end--;
		*str_end = 0;
	}

	void ClearLog()
	{
		buffer.clear();
		lineOffsets.clear();
		lineOffsets.push_back(0);
	}

	LogLevel GetLineLogLevel(const char* begin, const char* end) const
	{
		if (*begin != '[' || end - begin < 16) {
			if (*begin == '>') {
				return LogLevel::Info;
			}
			return cachedLineLevel;
		}
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

	void AddLog(const std::string& str)
	{
		int32 oldSize = static_cast<int32>(buffer.size());
		buffer.reserve(static_cast<int32>(oldSize + 2 + str.size()));
		buffer.append("\n");
		buffer.append(str.c_str());
		for (int newSize = buffer.size(); oldSize < newSize; oldSize++) {
			if (buffer[oldSize] == '\n') {
				lineOffsets.push_back(oldSize + 1);
			}
		}
	}

	void AddLogConsole(const std::string& log) { AddLogConsole(log.c_str()); }

	void AddLogConsole(const char* log)
	{
		int32 oldSize = static_cast<int32>(buffer.size());
		buffer.append("\n> ");
		buffer.append(log);
		for (int newSize = buffer.size(); oldSize < newSize; oldSize++) {
			if (buffer[oldSize] == '\n') {
				lineOffsets.push_back(oldSize + 1);
			}
		}
	}

	static constexpr ImVec4 c_warnColor = { 1.0f, 0.8f, 0.6f, 1.0f };
	static constexpr ImVec4 c_errColor = { 1.0f, 0.4f, 0.4f, 1.0f };

	void DrawTxtLine(const char* line_start, const char* line_end)
	{
		cachedLineLevel = GetLineLogLevel(line_start, line_end);
		switch (cachedLineLevel) {
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


	void DrawLog()
	{
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false,
			ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText

		bool copyToClipboard = false;
		if (ImGui::BeginPopupContextWindow("ContextClick##ConsoleWindow")) {
			if (ImGui::Checkbox("Auto-scroll", &autoScroll)) {
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Button("Clear")) {
				ClearLog();
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Button("Copy")) {
				copyToClipboard = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, -1.5f));
		ImEd::BeginCodeFont();


		if (copyToClipboard) {
			ImGui::LogToClipboard();
		}


		const char* buf = buffer.begin();
		const char* buf_end = buffer.end();
		if (filter.IsActive()) {
			// In this example we don't use the clipper when Filter is enabled.
			// This is because we don't have a random access on the result on our filter.
			// A real application processing logs with ten of thousands of entries may want to store the result
			// of search/filter. especially if the filtering function is not trivial (e.g. reg-exp).
			for (int line_no = 0; line_no < lineOffsets.Size; line_no++) {
				const char* line_start = buf + lineOffsets[line_no];
				const char* line_end
					= (line_no + 1 < lineOffsets.Size) ? (buf + lineOffsets[line_no + 1] - 1) : buf_end;

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
			clipper.Begin(lineOffsets.Size);
			while (clipper.Step()) {
				for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++) {
					const char* line_start = buf + lineOffsets[line_no];
					const char* line_end
						= (line_no + 1 < lineOffsets.Size) ? (buf + lineOffsets[line_no + 1] - 1) : buf_end;

					DrawTxtLine(line_start, line_end);
				}
			}
			clipper.End();
		}

		if (copyToClipboard) {
			ImGui::LogFinish();
		}

		ImEd::EndCodeFont();
		ImGui::PopStyleVar();

		if (scrollToBottom || (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())) {
			ImGui::SetScrollHereY(1.0f);
		}
		scrollToBottom = false;


		ImGui::EndChild();
	}

	void DrawConsole()
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Cmd: ");
		ImGui::SameLine();

		bool reclaim_focus = false;
		if (ImGui::InputText("##ConsoleTxtbox", inputBuf, IM_ARRAYSIZE(inputBuf),
				ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion
					| ImGuiInputTextFlags_CallbackHistory,
				&TextEditCallbackStub, (void*)this)) {
			char* s = inputBuf;
			Strtrim(s);
			if (s[0])
				ExecCommand(s);
			strcpy(s, "");
			reclaim_focus = true;
		}

		// Auto-focus on window apparition
		ImGui::SetItemDefaultFocus();
		if (reclaim_focus) {
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
		}
		ImEd::HSpace();
		ImGui::SameLine();
		ImGui::Text("Filter: ");
		ImGui::SameLine();
		filter.Draw("##LogFilter");
	}

	void Draw()
	{
		DrawConsole();
		ImGui::Separator();
		DrawLog();
	}

	void ExecCommand(const char* command_line)
	{
		// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to
		// be smart or optimal.
		historyPos = -1;
		for (int i = history.Size - 1; i >= 0; i--)
			if (Stricmp(history[i], command_line) == 0) {
				free(history[i]);
				history.erase(history.begin() + i);
				break;
			}
		history.push_back(Strdup(command_line));

		ProcessCommand(command_line);

		// On commad input, we scroll to bottom even if autoScroll==false
		scrollToBottom = true;
	}

	void ProcessCommand(const char* command)
	{
		// Forward the command to the backend
		Console::Execute(std::string_view(command));
	}

	static int TextEditCallbackStub(ImGuiInputTextCallbackData*
			data) // In C++11 you are better off using lambdas for this sort of forwarding callbacks
	{
		ConsoleState* console = (ConsoleState*)data->UserData;
		return console->TextEditCallback(data);
	}

	int TextEditCallback(ImGuiInputTextCallbackData* data)
	{
		// AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
		switch (data->EventFlag) {
			case ImGuiInputTextFlags_CallbackCompletion: {
				// Locate beginning of current word
				const char* word_end = data->Buf + data->CursorPos;
				const char* word_start = word_end;
				while (word_start > data->Buf) {
					const char c = word_start[-1];
					if (c == ' ' || c == '\t' || c == ',' || c == ';') {
						return 0;
					}

					word_start--;
				}

				// Build a list of candidates
				auto candidates = Console::AutoCompleteSuggest(std::string_view(word_start));

				if (candidates.size() == 0) {
					AddLogConsole("No match to autocomplete.");
				}
				else if (candidates.size() == 1) {
					// Single match. Delete the beginning of the word and replace it entirely so we've got nice
					// casing
					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
					data->InsertChars(data->CursorPos, candidates[0]->name);
					data->InsertChars(data->CursorPos, " ");
				}
				else {
					// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and
					// display "CLEAR" and "CLASSIFY"
					int match_len = (int)(word_end - word_start);
					for (;;) {
						int c = 0;
						bool all_candidates_matches = true;

						for (int i = 0; i < candidates.size() && all_candidates_matches; i++) {
							if (i == 0) {
								c = toupper(candidates[i]->name[match_len]);
							}
							else if (c == 0 || c != toupper(candidates[i]->name[match_len])) {
								all_candidates_matches = false;
							}
						}

						if (!all_candidates_matches) {
							break;
						}

						match_len++;
					}

					if (match_len > 0) {
						data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
						data->InsertChars(data->CursorPos, candidates[0]->name, candidates[0]->name + match_len);
					}

					AddLog(" ");
					for (int i = 0; i < candidates.size(); i++) {
						AddLogConsole(candidates[i]->GetDescriptionLine());
					}
				}

				break;
			}
			case ImGuiInputTextFlags_CallbackHistory: {
				// Example of HISTORY
				const int prev_history_pos = historyPos;
				if (data->EventKey == ImGuiKey_UpArrow) {
					if (historyPos == -1)
						historyPos = history.Size - 1;
					else if (historyPos > 0)
						historyPos--;
				}
				else if (data->EventKey == ImGuiKey_DownArrow) {
					if (historyPos != -1)
						if (++historyPos >= history.Size)
							historyPos = -1;
				}

				// A better implementation would preserve the data on the current input line along with cursor
				// position.
				if (prev_history_pos != historyPos) {
					const char* history_str = (historyPos >= 0) ? history[historyPos] : "";
					data->DeleteChars(0, data->BufTextLen);
					data->InsertChars(0, history_str);
				}
			}
		}
		return 0;
	}
};
} // namespace

namespace ed {

void ConsoleWindow::OnDraw(const char* title, bool* keepOpen)
{
	static ConsoleState console;
	auto& ss = Log.s_editorLogStream;

	std::string line;
	while (std::getline(ss, line)) {
		console.AddLog(line.c_str());
	}
	ss.clear();

	ImGui::SetNextWindowPos(ImVec2(700.f, 300.f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(title, keepOpen)) {
		console.Draw();
	}
	ImGui::End();
}
} // namespace ed
