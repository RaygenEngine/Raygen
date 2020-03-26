#pragma once
#include "editor/EdInput.h"
#include "engine/Listener.h"

#include <concepts>
#include <string>

// Code example for imgui drawing:
//
// void MyWindow::OnDraw(const char* title, bool* keepOpen)
// {
//	 if (ImGui::Begin(title, keepOpen)) {
//	 	ImGui::Text("Hello World!");
//	 }
//	 ImGui::End();
// }


namespace ed {
// Base Window / Tab to be used in the editor as a view
class Window : public Listener {


protected:
	// The interal title passed to imgui, acts as cache to avoid allocations
	std::string m_fullTitle{};

	std::string m_identifier{};

	// The visible part of the title
	std::string m_title{};

	bool m_keepOpen{ true };

protected:
	void ReformatTitle();
	Window(std::string_view title, std::string_view identifier = "");

public:
	// Internal use only. Returns false when a close has been requested internally. (ie: user clicks X, or code
	// behavior)
	bool Z_Draw();

	// TODO: Not called (unused) Implement these to finish the interface
	virtual void OnOpen() {}
	virtual void OnClose() {}

	// Overwrite this and write your imgui drawing here. Given parameters are defaults to be passed to ImGui::Begin.
	// Prefer ImguiDraw.
	virtual void OnDraw(const char* title, bool* keepOpen);

	// Overwrite this INSTEAD of OnDraw if you don't want to do anything special with the windows. No Need to
	// ImGui::Begin from here
	virtual void ImguiDraw(){};


	// TODO: Editor shortcuts
	virtual bool HandleShortcut(KeyShortcut) { return false; };

	// Mark this window is to be closed
	void MarkClose() { m_keepOpen = false; }

	// CHECK: currently this will make the window "jump" if the identifier is not a ### one. (Unique windows unaffected)
	void SetTitle(std::string_view newTitle)
	{
		m_title = newTitle;
		ReformatTitle();
	}

	virtual ~Window() = default;
};

// A unique window that has a "unique" instance open at most at the same time.
// Useful for most cases where the window is not a document editor, also allows easy registration to the editor.
class UniqueWindow : public Window {

public:
	UniqueWindow(std::string_view initialTitle);


	virtual ~UniqueWindow() = default;
};


class MultiWindow : public Window {
public:
	MultiWindow(std::string_view title, std::string_view identifier = "");

	void SetIdentifier(std::string_view newIdentifier)
	{
		m_identifier = newIdentifier;
		ReformatTitle();
	}

	virtual ~MultiWindow() = default;
};

template<class T>
concept WindowClass = std::derived_from<T, Window>;

template<class T>
concept UniqueWindowClass = std::derived_from<T, UniqueWindow>;

template<class T>
concept MultiWindowClass = std::derived_from<T, MultiWindow>;

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
