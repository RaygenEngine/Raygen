
#include "editor/imgui/ImguiImpl.h"
#include "system/Engine.h"
#include "platform/windows/Win32Window.h"
#define IMGUI_IMPL_OPENGL_LOADER_GLAD

#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_win32.h>

// forward declare this in our own file because its commented out in the imgui impl header.
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace imguisyle {
void SetStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();

	// If you want to edit this, use ImGui::ShowStyleEditor for an actual editor.

	style.WindowPadding = ImVec2(6.f, 3.f);
	style.FramePadding = ImVec2(3.f, 3.f);
	style.ItemSpacing = ImVec2(2.f, 2.f);
	style.ItemInnerSpacing = ImVec2(3.f, 3.f);
	style.TouchExtraPadding = ImVec2(3.f, 1.f);
	style.IndentSpacing = 14.f;
	style.ScrollbarSize = 15.f;
	// style.GrabMinSize default

	style.WindowBorderSize = 0.f;
	style.ChildBorderSize = 1.f;
	style.PopupBorderSize = 1.f;
	style.FrameBorderSize = 0.f;
	style.TabBorderSize = 0.f;

	style.WindowRounding = 0.f;
	style.ChildRounding = 0.f;
	style.FrameRounding = 0.f;
	style.PopupRounding = 0.f;
	style.ScrollbarRounding = 0.f;
	style.GrabRounding = 0.f;
	style.TabRounding = 0.f;


	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg].w = 0.98f;
	colors[ImGuiCol_Text] = ImVec4(0.85f, 0.85f, 0.89f, 0.98f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 0.8f);

	// Static this, needs to be valid for as long as imgui is used.
	static const ImWchar ranges[] = { 0x0007, 0x00FF, 0 };

	ImGui::GetIO().Fonts->AddFontFromFileTTF("engine-data/fonts/UbuntuMedium.ttf", 15, nullptr, ranges);
	ImGui::GetIO().Fonts->AddFontFromFileTTF("engine-data/fonts/UbuntuMonoRegular.ttf", 14, nullptr, ranges);

	ImGui::GetIO().Fonts->Build();
} // namespace imguisyle
} // namespace imguisyle
void ImguiImpl::InitContext()
{
	if (!Engine::GetMainWindow()) {
		LOG_ERROR("Failed to load imgui, window not created yet. Please make a main window before imgui init.");
		return;
	}
	ImGui::CreateContext();

	ImGui::StyleColorsDark();
	imguisyle::SetStyle();

	ImGui_ImplWin32_Init(Engine::GetMainWindow()->GetHWND());
	ImGui::GetIO().IniFilename = "EditorImgui.ini";
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
	if (ImGui::GetCurrentContext() != nullptr) {
		if (ImGui::GetIO().WantCaptureKeyboard) {
			switch (message) {
				case WM_KEYDOWN:
				case WM_SYSKEYDOWN: return true;
			}
		}
		if (ImGui::GetIO().WantCaptureMouse) {
			switch (message) {
				case WM_MOUSEWHEEL:
				case WM_MOUSEHWHEEL:
				case WM_LBUTTONDOWN:
				case WM_RBUTTONDOWN:
				case WM_MBUTTONDOWN:
				case WM_XBUTTONDOWN:
				case WM_LBUTTONDBLCLK:
				case WM_RBUTTONDBLCLK:
				case WM_MBUTTONDBLCLK:
				case WM_XBUTTONDBLCLK: return true;
			}
		}

		return false;
	}

	return false;
}

#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_widgets.cpp"
#include "imgui/imgui_impl_opengl3.cpp"
#include "imgui/imgui_impl_win32.cpp"
#include "imgui/imgui_stdlib.cpp"
