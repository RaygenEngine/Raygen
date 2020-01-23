#pragma once

#include <windows.h>

// Provides an abstraction over the specific Win32 + ogl imgui implementation
class ImguiImpl {
public:
	static void InitContext();
	static void CleanupContext();

	static void NewFrame();
	static void EndFrame();

	static LRESULT WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
