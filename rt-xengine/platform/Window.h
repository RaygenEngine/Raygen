#ifndef WINDOW_H
#define WINDOW_H


#include <system/Engine.h>

namespace Platform
{

// Base class for a platform independant window
class Window
{
protected:
	bool m_focused;
	bool m_closed;

	int m_width;
	int m_height;

	// Non owning pointer to the engine object.
	System::Engine* m_engineRef;

public:

	// Attach input before creating window
	Window(System::Engine* engineRef)
		: m_engineRef(engineRef),
		  m_focused(false),
		  m_closed(false), m_width(0), m_height(0)
	{ };

	virtual ~Window() = default;

	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }

	bool IsClosed() const { return m_closed; }
	bool IsFocused() const { return m_focused; }

	virtual void RestrictMouseMovement() { }
	virtual void ReleaseMouseMovement() { }

	virtual void Show() { }
	
	virtual bool StartRenderer(System::RendererRegistrationIndex index) { return false; }

	virtual void SetTitle(const std::string& newTitle) { };

	// Called in the main loop
	virtual void HandleEvents(bool shouldHandleControllers) { };
};

}

#endif // WINDOW_H
