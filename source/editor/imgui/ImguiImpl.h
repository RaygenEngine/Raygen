#pragma once

#include <windows.h>

// Provides an abstraction over the specific Win32 + OpenGL imgui implementation
class ImguiImpl {
public:
	static void InitContext();
	static void CleanupContext();

	static void NewFrame();
	static void EndFrame();

	static void InitOpenGL();
	static void RenderOpenGL();
	static void CleanupOpenGL();

	static LRESULT WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};