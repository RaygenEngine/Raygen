#pragma once

#include <windows.h>

// Provides an abstraction over the specific Win32 + OpenGL imgui implementation
class ImguiImpl 
{
public:

	static void Init(HWND hWnd);
	static void NewFrame();
	static void EndFrame();
	static void OpenGLRender();
	static void Cleanup();

	static LRESULT WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};