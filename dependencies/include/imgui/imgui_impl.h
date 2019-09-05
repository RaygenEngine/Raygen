#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_opengl3.h"

void ImGui_Impl_Init();
void ImGui_Impl_NewFrame();
void ImGui_Impl_EndFrame();
void ImGui_Impl_Cleanup();

