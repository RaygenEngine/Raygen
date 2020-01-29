#pragma once
#include "editor/EdInput.h"
#include "imgui/imgui.h"
#include <string>
#include <concepts>

namespace ed {
// Base Window / Tab to be used in the editor as a view
class Window {

	static uint64 s_windowId;
	uint64 m_id;

protected:
	bool m_isOpen{ false };

	std::string m_title{};

	ImGuiWindowFlags m_flags{};

public:
	Window(std::string_view title);
	void Z_Draw();

	virtual void OnOpen() {}
	virtual void OnDraw() {}
	virtual void OnClose() {}

	virtual bool HandleShortcut(KeyShortcut) { return false; };

	void SetTitle(const std::string& newTitle) { m_title = newTitle + "###" + std::to_string(m_id); }

	virtual ~Window() = default;
};


template<class T>
concept WindowClass = std::derived_from<T, Window>;


/*
class DocumentWindow : public Window {
	bool m_needsSave{ false };

public:
	void MarkUnsaved()
	{
		m_needsSave = true;
		m_flags |= ImGuiWindowFlags_UnsavedDocument;
	}

	void ClearUnsaved()
	{
		m_needsSave = false;
		m_flags &= ~(ImGuiWindowFlags_UnsavedDocument);
	}


	virtual bool OnSave() {}
};
*/
} // namespace ed
