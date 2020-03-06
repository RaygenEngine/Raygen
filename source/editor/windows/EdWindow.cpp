#include "pch.h"
#include "editor/windows/EdWindow.h"
#include "engine/Logger.h"
#include "imgui/imgui.h"

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
	if (ImGui::Begin(title, keepOpen)) {
		ImguiDraw();
	}
	ImGui::End();
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

MultiWindow::MultiWindow(std::string_view title, std::string_view identifier)
	: Window(title, identifier)
{
}
} // namespace ed
