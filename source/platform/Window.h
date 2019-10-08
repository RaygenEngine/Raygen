#pragma once
#include "system/EngineComponent.h"
#include "event/Event.h"

// Base class for a platform independant window
class Window {
protected:
	bool m_focused{ false };
	bool m_closed{ false };

	int m_width{ 0 };
	int m_height{ 0 };

public:
	virtual ~Window() = default;

	[[nodiscard]] int GetWidth() const { return m_width; }
	[[nodiscard]] int GetHeight() const { return m_height; }

	[[nodiscard]] bool IsClosed() const { return m_closed; }
	[[nodiscard]] bool IsFocused() const { return m_focused; }

	virtual void RestrictMouseMovement() {}
	virtual void ReleaseMouseMovement() {}

	virtual void Show() {}
	virtual void Hide() {}

	// Expected to fire the engine's resize event before the first window->Show(),
	// allows the renderers to init with correct camera sizes before the window is visible.
	virtual void FireFirstResizeEvent() {}

	virtual void SetTitle(const std::string& newTitle){};

	// Called in the main loop
	virtual void HandleEvents(bool shouldHandleControllers){};
};
