#include "ImguiImpl.h"
#include "system/Engine.h"
#include "platform/windows/Win32Window.h"
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"

// forward declare this in our own file because its commented out in the imgui impl header.
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void ImguiImpl::InitContext()
{
	if (!Engine::GetMainWindow()) {
		LOG_ERROR("Failed to load imgui, window not created yet. Please make a main window before imgui init.");
		return;
	}
	ImGui::CreateContext();

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(Engine::GetMainWindow()->GetHWND());
	ImGui::GetIO().IniFilename = "scenes/imgui.ini";
}

void ImguiImpl::NewFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImguiImpl::EndFrame()
{
	ImGui::EndFrame();
	ImGui::Render();
}

void ImguiImpl::CleanupContext()
{
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImguiImpl::InitOpenGL()
{
	ImGui_ImplOpenGL3_Init();
}

void ImguiImpl::RenderOpenGL()
{
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImguiImpl::CleanupOpenGL()
{
	ImGui_ImplOpenGL3_Shutdown();
}

LRESULT ImguiImpl::WndProcHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Its possible to actually implement this over the engine's input system
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam)) {
		return true;
	}

	// Imgui handles keys but also forwards them so we manually check if we should forward them to the main window
	// handler.
	if (ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureKeyboard
		&& (message == WM_KEYDOWN || message == WM_SYSKEYDOWN || message == WM_KEYUP || message == WM_SYSKEYUP)) {
		return true;
	}

	return false;
}

// TODO: when transfered to the engine side make this a static lib generated seperately

#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_widgets.cpp"
#include "imgui/imgui_impl_opengl3.cpp"
#include "imgui/imgui_impl_win32.cpp"
#include "imgui/imgui_stdlib.cpp"
