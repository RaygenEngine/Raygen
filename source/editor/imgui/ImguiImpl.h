#pragma once

struct ImFont;

class ImguiImpl {
public:
	static void InitContext();
	static void CleanupContext();

	static void NewFrame();
	static void EndFrame();

	static void RenderDX12(ID3D12GraphicsCommandList* d3d12CommandList);

	inline static ImFont* s_EditorFont{ nullptr };
	inline static ImFont* s_CodeFont{ nullptr };
	inline static ImFont* s_AssetIconFont{ nullptr };
	inline static ImFont* s_MediumSizeIconFont{ nullptr };


	static std::pair<glm::vec2, glm::vec2> GetIconUV(const char* icon);

	// vk::DescriptorSet GetIconFontDescriptorSet() NEW::
};
