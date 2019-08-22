#ifndef WIN32WINDOW_H
#define WIN32WINDOW_H

#include <windows.h>

namespace Platform
{
	class Win32Window 
	{
		WNDCLASSEX m_wcex;

		HWND m_hWnd;

		bool m_focused;
		bool m_closed;

		int32 m_width;
		int32 m_height;

		DWORD m_controllerIndex;

	public:

		// Attach input before creating window
		Win32Window();
		~Win32Window();

		// Window class styles. This is a combination of one or more Window Class Styles
		// Window class name. This class name is used by several functions to retrieve window information at run time.
		// Window background brush color. A brush handle representing the background color.
		// Window cursor
		// Window procedure associated to this window class. A pointer to a window procedure function. This function is used for message processing.
		// The application instance
		bool Register(
			UINT style,
			LPCSTR name,
			HBRUSH backgroundBrushColor,
			HCURSOR cursor,
			HINSTANCE instance);

		bool Create(
			int32 xpos, 
			int32 ypox, 
			int32 width, 
			int32 height, 
			LPCSTR title, 
			LONG style);

		int32 GetWidth() const { return m_width; }
		int32 GetHeight() const { return m_height; }

		HWND GetHWND() const { return m_hWnd; }

		HINSTANCE GetHInstance() const { return m_wcex.hInstance; }

		bool IsClosed() const { return m_closed; }
		bool IsFocused() const { return m_focused; }

		void RestrictMouseMovement();
		void ReleaseMouseMovement();

		void Display();

		friend LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	};

}

#endif // WIN32WINDOW_H
