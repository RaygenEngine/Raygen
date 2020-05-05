#include "pch.h"
#include "EdWindow.h"

#include "engine/Logger.h"
#include "assets/AssetManager.h"
#include "reflection/ReflClass.h"

#include <imgui/imgui.h>

namespace ed {

Window::Window(std::string_view title, std::string_view identifier)
{
	m_title = title;
	m_identifier = identifier;
	ReformatTitle();
}

bool Window::Z_Draw()
{
	OnDraw(m_fullTitle.c_str(), &m_keepOpen);

	if (!m_keepOpen) {
		m_keepOpen = true;
		return false;
	}
	return true;
}

void Window::OnDraw(const char* title, bool* keepOpen)
{
	ImGui::SetNextWindowSize({ 400, 400 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos({ 400, 250 }, ImGuiCond_FirstUseEver);
	if (ImGui::Begin(title, keepOpen)) {
		ImguiDraw();
	}
	ImGui::End();
}

void Window::BringToFront()
{
	ImGui::SetWindowFocus(m_fullTitle.c_str());
}

void Window::ReformatTitle()
{
	if (m_identifier.size() > 0) {
		m_fullTitle = fmt::format("{}##{}", m_title, m_identifier);
	}
	else {
		m_fullTitle = m_title;
	}
	m_title = m_fullTitle.c_str();
}

UniqueWindow::UniqueWindow(std::string_view initialTitle)
	: Window(initialTitle)
{
	m_identifier = fmt::format("#{}_Unq", initialTitle);
	ReformatTitle();
}

AssetEditorWindow::AssetEditorWindow(PodEntry* inEntry)
	: Window(inEntry->name, inEntry->path)
	, entry(inEntry)
{
}

void AssetEditorWindow::ImguiDraw()
{
	std::string txt = fmt::format(
		"No editor found for this pod type: {}\nPod for edit: {}", entry->GetClass()->GetName(), entry->path);
	ImGui::Text(txt.c_str());
}

void AssetEditorWindow::SaveToDisk()
{
	AssetHandlerManager::SaveToDisk(entry);
}
} // namespace ed
