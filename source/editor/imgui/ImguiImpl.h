#pragma once

namespace vk {
class CommandBuffer;
} // namespace vk

struct ImFont;

// NEXT: singleton
class ImguiImpl {
public:
	static void InitContext();
	static void CleanupContext();

	static void NewFrame();
	static void EndFrame();

	static void InitVulkan();
	static void RenderVulkan(vk::CommandBuffer* drawCommandBuffer);

	inline static ImFont* s_EditorFont{ nullptr };
	inline static ImFont* s_CodeFont{ nullptr };
	inline static ImFont* s_AssetIconFont{ nullptr };
};
