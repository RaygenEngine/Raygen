#pragma once

namespace vk {
class CommandBuffer;
} // namespace vk

struct ImFont;

using ImTextureID = void*;

class ImguiImpl {
public:
	static void InitContext();
	static void CleanupContext();

	static void NewFrame();
	static void EndFrame();

	static void RenderVulkan(vk::CommandBuffer* drawCommandBuffer);

	inline static ImFont* s_EditorFont{ nullptr };
	inline static ImFont* s_CodeFont{ nullptr };
	inline static ImFont* s_AssetIconFont{ nullptr };
	inline static ImFont* s_MediumSizeIconFont{ nullptr };

	static std::pair<glm::vec2, glm::vec2> GetIconUV(const char* icon);

	static ImTextureID GetFontIconTexture();
};
