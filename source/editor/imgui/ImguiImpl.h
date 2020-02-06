#pragma once

#include <windows.h>
namespace vk {
class CommandBuffer;
} // namespace vk

// Provides an abstraction over the specific Win32 + ogl imgui implementation
class ImguiImpl {
public:
	static void InitContext();
	static void CleanupContext();

	static void NewFrame();
	static void EndFrame();

	static void InitOpenGL();
	static void RenderOpenGL();
	static void CleanupOpenGL();


	static void InitVulkan();
	static void RenderVulkan(vk::CommandBuffer* drawCommandBuffer);
	static void CleanupVulkan();


	static LRESULT WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
