#include "pch/pch.h"
#include "editor/windows/EdWindow.h"
#include "system/Logger.h"
namespace ed {

uint64 Window::s_windowId = 1;

Window::Window(std::string_view title)
{
	{
		m_id = s_windowId++;
		m_title = fmt::format("{}###{}", title, m_id);
	}
}

void Window::Z_Draw()
{
	bool prevOpen = m_isOpen;

	bool shouldDraw = ImGui::Begin(m_title.c_str(), &m_isOpen, m_flags);

	if (prevOpen != m_isOpen) {
		// TODO: Check unsaved windows drawing behavior
		m_isOpen ? OnOpen() : OnClose();
	}

	if (shouldDraw) {
		OnDraw();
	}

	ImGui::End();
}
} // namespace ed
