#pragma once

// Base class for a platform independant window
class Window {
protected:
	bool m_focused;
	bool m_closed;

	int m_width;
	int m_height;

public:
	// Attach input before creating window
	Window()
		: m_focused(false)
		, m_closed(false)
		, m_width(0)
		, m_height(0)
	{
	}

	virtual ~Window() = default;

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }

	bool IsClosed() const { return m_closed; }
	bool IsFocused() const { return m_focused; }

	virtual void Destroy() = 0;

	virtual void RestrictMouseMovement() {}
	virtual void ReleaseMouseMovement() {}
	virtual void DrawSplash(){};
	virtual void DrawLoading(){};

	virtual void Show() = 0;
	virtual void Hide() = 0;


	// Expected to fire the engine's resize event before the first window->Show(),
	// allows the renderers to init with correct camera sizes before the window is visible.
	virtual void FireFirstResizeEvent() = 0;

	virtual void SetTitle(const std::string& newTitle){};

	// Called in the main loop
	virtual void HandleEvents(bool shouldHandleControllers) = 0;
};
