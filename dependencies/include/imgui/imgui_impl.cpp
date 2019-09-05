#include "imgui/imgui_impl.h"

void ImGui_Impl_Init()
{
	ImGui::CreateContext();
	ImGui::StyleColorsLight();
	
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplOpenGL3_Init();
}

void ImGui_Impl_NewFrame()
{
			ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
}
void ImGui_Impl_EndFrame();
void ImGui_Impl_Cleanup();