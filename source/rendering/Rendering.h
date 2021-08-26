#pragma once

struct ImGui_ImplVulkan_InitInfo;
struct ImDrawData;
struct Scene;
namespace vl {
class RendererBase;
}
using ImTextureID = void*;


namespace vk {
class CommandBuffer;
} // namespace vk

class Rendering {
	friend class Engine_;
	static void Init();
	static void Destroy();

public:
	static void DrawFrame();
	static Scene* GetMainScene();
	static vl::RendererBase* GetActiveRenderer();

	static void ResetMainScene();

	static bool IsPathtracing();

	static void Imgui_Prepare();
	static void Imgui_Shutdown();
	static void Imgui_NewFrame();
	static void Imgui_DrawFrame(ImDrawData* drawData, vk::CommandBuffer* drawCommandBuffer);

	static void SwapRenderingMode();
};
