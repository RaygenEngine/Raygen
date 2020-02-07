#pragma once

namespace vk {
class CommandBuffer;
} // namespace vk

class ImguiImpl {
public:
	static void InitContext();
	static void CleanupContext();

	static void NewFrame();
	static void EndFrame();

	static void InitVulkan();
	static void RenderVulkan(vk::CommandBuffer* drawCommandBuffer);
};
