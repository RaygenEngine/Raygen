#include "pch.h"

#include "platform/windows/Win32Window.h"
#include "platform/windows/Win32App.h"
#include "platform/windows/TranslateWin32VirtualKeys.h"

#include <windowsx.h>

namespace Platform
{
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	Win32Window::Win32Window()
		: m_wcex(),
		  m_hWnd(nullptr),
		  m_focused(false),
		  m_closed(false), m_width(0), m_height(0)
	{
	}

	Win32Window::~Win32Window()
	{
		// Unregister window class, freeing the memory that was
		// previously allocated for this window.

		::UnregisterClass(m_wcex.lpszClassName, m_wcex.hInstance);
	}

	bool Win32Window::Register(
		UINT style,
		LPCSTR name,
		HBRUSH backgroundBrushColor,
		HCURSOR cursor,
		HINSTANCE instance)
	{
		// Setup window class attributes.
		ZeroMemory(&m_wcex, sizeof(m_wcex));

		m_wcex.cbSize = sizeof(m_wcex);
		m_wcex.style = style;
		m_wcex.lpszClassName = name;
		m_wcex.hbrBackground = backgroundBrushColor;
		m_wcex.hCursor = cursor;
		m_wcex.lpfnWndProc = WndProc;
		m_wcex.hInstance = instance;

		// Register window and ensure registration success.
		return static_cast<bool>(RegisterClassEx(&m_wcex));
	}

	bool Win32Window::Create(
		int32 xpos, 
		int32 ypox, 
		int32 width, 
		int32 height, 
		LPCSTR title, 
		LONG style)
	{
		// Setup window initialization attributes.
		CREATESTRUCT cs;
		ZeroMemory(&cs, sizeof(cs));

		cs.x = xpos;	                       // Window X position
		cs.y = ypox;	                       // Window Y position
		cs.cx = width;	                       // Window width
		cs.cy = height;	                       // Window height
		cs.hInstance = m_wcex.hInstance;       // Window instance.
		cs.lpszClass = m_wcex.lpszClassName;   // Window class name
		cs.lpszName = title;				   // Window title
		cs.style = style;					   // Window style

		// Create the window.
		m_hWnd = ::CreateWindowEx(
			cs.dwExStyle,
			cs.lpszClass,
			cs.lpszName,
			cs.style,
			cs.x,
			cs.y,
			cs.cx,
			cs.cy,
			cs.hwndParent,
			cs.hMenu,
			cs.hInstance,
			cs.lpCreateParams);

		return m_hWnd;
	}

	void Win32Window::RestrictMouseMovement()
	{
		// restrict mouse inside window
		RECT rect;
		GetClientRect(m_hWnd, &rect);

		POINT ul = { rect.left,rect.top };
		POINT lr = { rect.right, rect.bottom };

		MapWindowPoints(m_hWnd, nullptr, &ul, 1);
		MapWindowPoints(m_hWnd, nullptr, &lr, 1);

		rect = { ul.x, ul.y, lr.x, lr.y };
		ClipCursor(&rect);
	}

	void Win32Window::ReleaseMouseMovement()
	{
		ClipCursor(NULL);
	}

	void Win32Window::Display()
	{
		::ShowWindow(m_hWnd, SW_SHOWDEFAULT);
		::UpdateWindow(m_hWnd);
	}

	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		LRESULT result = NULL;

		auto* app = reinterpret_cast<Win32App*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		if(!app) return DefWindowProc(hWnd, message, wParam, lParam);

		auto& input = app->m_engine->GetInput();

		switch (message)
		{
		case WM_SETFOCUS:
			SetFocus(hWnd);
			app->m_window->m_focused = true;
			break;

		case WM_KILLFOCUS:
			app->m_window->m_focused = false;
			break;

		case WM_CLOSE:
		case WM_DESTROY:
			app->m_window->m_closed = true;
			PostQuitMessage(0);
			break;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			// escape loses focus - use to resize, close, etc.
			// also propagated in input
			if (wParam == VK_ESCAPE)
				app->m_window->ReleaseMouseMovement();

			// translate generic key press 
			input.UpdateKeyPressed(TranslateWin32VirtualKeys(wParam));
			// map extended key press
			if(MapLeftRightKeys(wParam, lParam))
				input.UpdateKeyPressed(TranslateWin32VirtualKeys(wParam));
			break;

		case WM_KEYUP:
		case WM_SYSKEYUP:
			// translate generic key up 
			input.UpdateKeyReleased(TranslateWin32VirtualKeys(wParam));
			// map extended key up
			if (MapLeftRightKeys(wParam, lParam))
				input.UpdateKeyReleased(TranslateWin32VirtualKeys(wParam));
			break;

		case WM_LBUTTONDOWN:
			input.UpdateKeyPressed(XVK_LBUTTON);
			break;

		case WM_RBUTTONDOWN:
			input.UpdateKeyPressed(XVK_RBUTTON);
			break;

		case WM_MBUTTONDOWN:
			input.UpdateKeyPressed(XVK_MBUTTON);
			break;

		case WM_XBUTTONDOWN:
			input.UpdateKeyPressed(TranslateWin32VirtualKeys(MapExtraMouseButtons(GET_XBUTTON_WPARAM(wParam))));
			break;

		case WM_LBUTTONUP:
			input.UpdateKeyReleased(XVK_LBUTTON);
			break;

		case WM_RBUTTONUP:
			input.UpdateKeyReleased(XVK_RBUTTON);
			break;

		case WM_MBUTTONUP:
			input.UpdateKeyReleased(XVK_MBUTTON);
			break;

		case WM_XBUTTONUP:
			input.UpdateKeyReleased(TranslateWin32VirtualKeys(MapExtraMouseButtons(GET_XBUTTON_WPARAM(wParam))));
			break;

		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_XBUTTONDBLCLK:
			app->m_window->RestrictMouseMovement();
			input.UpdateDoubleClick();
			break;

		case WM_MOUSEWHEEL:
			input.UpdateWheel(GET_WHEEL_DELTA_WPARAM(wParam));
			break;

		case WM_MOUSEMOVE:
			// drag is considered any of the mouse buttons as modifier
			if (wParam & MK_LBUTTON ||
				wParam & MK_MBUTTON ||
				wParam & MK_RBUTTON ||
				wParam & MK_XBUTTON1 ||
				wParam & MK_XBUTTON2)
				input.UpdateCursorDrag();

			input.UpdateCursorPosition({GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)});
			break;

		case WM_SIZE:
			// don't report ever 0 width/height to avoid numerical errors
			if (LOWORD(lParam) == 0 || HIWORD(lParam) == 0)
				break;

			app->m_window->m_width = LOWORD(lParam);
			app->m_window->m_height = HIWORD(lParam);

			app->m_engine->GetWorld()->WindowResize(LOWORD(lParam), HIWORD(lParam));
			app->m_engine->GetRenderer()->WindowResize(LOWORD(lParam), HIWORD(lParam));
			break;

		default:
			result = DefWindowProc(hWnd, message, wParam, lParam);
		}

		return result;
	}

}
