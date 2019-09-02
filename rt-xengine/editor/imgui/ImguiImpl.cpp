#include "ImguiImpl.h"
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_win32.h"

// forward declare this in our own file because its commented out in the imgui impl header.
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


void ImguiImpl::Init(HWND hWnd)
{
	ImGui::CreateContext();

	ImGui::StyleColorsLight();

	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplOpenGL3_Init();
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

void ImguiImpl::OpenGLRender()
{
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImguiImpl::Cleanup()
{
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

LRESULT ImguiImpl::WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

// TODO: when transfered to the engine side make this a static lib generated seperately

#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_widgets.cpp"
#include "imgui/imgui_impl_opengl3.cpp"
#include "imgui/imgui_impl_win32.cpp"
#include "imgui/imgui_stdlib.cpp"
