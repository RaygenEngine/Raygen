#include "pch/pch.h"
#include "editor/imgui/FileBrowser.h"

#include "system/Logger.h"
#include "editor/imgui/ImEd.h"
#include "editor/TextIcons.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <algorithm>
namespace ImEd {
FileBrowser::Entry::Entry(const fs::directory_entry& e)
	: fsEntry(e)
	, path(e.path())
{
	if (e.is_directory()) {
		visibleName = fmt::format("{}  {}", U8(FA_FOLDER), U8(e.path().filename().u8string().c_str()));
	}
	else {
		visibleName = U8(e.path().filename().u8string().c_str());
	}
}

void FileBrowser::OpenDialog(std::function<void(const fs::path&)>&& callback, BrowserOperationInfo&& openInfo)
{
	m_isOpen = true;
	m_opInfo = std::move(openInfo);

	m_callback = std::move(callback);

	const fs::path& directory = m_opInfo.initialPath.empty() ? fs::current_path() : m_opInfo.initialPath;
	ChangeDirectory(directory);
}

void FileBrowser::ImguiHeader()
{
	std::string s = fmt::format("{}  ..", U8(FA_REPLY));

	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, !m_parentDirEntry.has_value());
	if (ImEd::Button(s.c_str())) {
		ChangeDirectory(m_parentDirEntry.value().path);
	}
	ImGui::PopItemFlag();

	ImGui::SameLine();
	if (ImEd::Button(U8(FA_REDO_ALT))) {
		RefreshDirectory();
	}

	float height = ImGui::GetItemRectSize().y;
	ImGui::SameLine();
	if (ImEd::InputTextSized(
			"##PathTxt", &m_currentPathTransient, ImVec2(-1.f, height), ImGuiInputTextFlags_EnterReturnsTrue)) {
		ChangeDirectory(fs::path(m_currentPathTransient));
	}

	static std::string folderName = "New Folder";
	const bool justOpened = ImEd::Button(U8(FA_FOLDER_PLUS u8"  New Folder"));
	if (justOpened) {
		ImGui::OpenPopup("FileBrowser_NewFolder_Popup");
		folderName = "New Folder";
	}


	if (ImGui::BeginPopup("FileBrowser_NewFolder_Popup", ImGuiWindowFlags_Popup)) {
		if (justOpened) {
			ImGui::SetKeyboardFocusHere();
		}
		bool create = ImGui::InputText("##NewFolderName", &folderName, ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		create |= ImGui::Button("Create");

		if (create) {
			// WIP
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void FileBrowser::Draw()
{
	if (!m_isOpen) {
		return;
	}
	else {
		ImGui::OpenPopup(m_opInfo.title.c_str());
	}

	ImGui::SetNextWindowSize(ImVec2(1400.f, 750.f), ImGuiCond_Appearing);
	if (!ImGui::BeginPopupModal(m_opInfo.title.c_str(), &m_isOpen, ImGuiWindowFlags_Modal)) {
		return;
	}

	ImGui::PushID(this);

	ImguiHeader();

	ImGui::BeginChild("FileList", ImVec2(0, -ImGui::GetTextLineHeightWithSpacing()), true);


	ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
	{
		for (auto& dir : m_directories) {
			if (DrawElement(dir)) {
				ChangeDirectory(dir.path);
				break;
			}
		}

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.2f, 0.2f, 1.0f));
		for (auto& file : m_files) {
			if (DrawElement(file)) {
				ConfirmSelection(file.path);
				break;
			}
		}
		ImGui::PopStyleColor();
	}
	ImGui::PopStyleVar(2);

	ImGui::EndChild();
	ImGui::Text(m_path.string().c_str());
	ImGui::PopID();
	ImGui::EndPopup();
}

bool FileBrowser::DrawElement(const Entry& entry)
{
	return ImGui::Button(entry.visibleName.c_str(), ImVec2(-1.f, 0));
}

void FileBrowser::ChangeDirectory(const fs::path& newPath)
{
	auto prevPath = m_path;
	m_path = newPath;
	m_currentPathTransient = m_path.string();

	m_directories.clear();
	m_files.clear();

	if (m_path.has_parent_path()) {
		m_parentDirEntry = Entry(fs::directory_entry(m_path.parent_path()));
	}
	else {
		m_parentDirEntry.reset();
	}

	try {
		for (auto& entry : fs::directory_iterator(m_path)) {
			if (entry.is_directory()) {
				m_directories.emplace_back(entry);
				continue;
			}

			if (!m_opInfo.fileFilters.empty()) {
				for (auto& ext : m_opInfo.fileFilters) {
					if (entry.path().extension() == ext) {
						m_files.emplace_back(entry);
					}
				}
			}
			else {
				m_files.emplace_back(entry);
			}
		}
	} catch (std::exception e) {
		LOG_ERROR("Failed to open a folder in the file browser. Exception was:\n{}", e.what());
		// TODO: possible stack overflow (rare) can happen here, when the directory we came from is inaccessible
		ChangeDirectory(prevPath);
	}
}

void FileBrowser::ConfirmSelection(const fs::path& path)
{
	ImGui::CloseCurrentPopup();
	m_isOpen = false;

	m_callback(path);
}

void FileBrowser::RefreshDirectory()
{
	ChangeDirectory(m_path);
}

} // namespace ImEd
