#pragma once
#include "system/EngineComponent.h"
#include "event/Event.h"

// Base class for a platform independant window
class Window
{
protected:
	bool m_focused;
	bool m_closed;

	int m_width;
	int m_height;

public:

	
	// Attach input before creating window
	Window()
		: m_focused(false),
		  m_closed(false), 
		  m_width(0), 
		  m_height(0) {}

	virtual ~Window() = default;

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }

	bool IsClosed() const { return m_closed; }
	bool IsFocused() const { return m_focused; }

	virtual void RestrictMouseMovement() { }
	virtual void ReleaseMouseMovement() { }

	virtual void Show() { }
	
	virtual void SetTitle(const std::string& newTitle) { };

	// Called in the main loop
	virtual void HandleEvents(bool shouldHandleControllers) { };
};