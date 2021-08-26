#pragma once
#include "engine/Listener.h"
#include "rendering/wrappers/CmdBuffer.h"

struct Scene;

namespace vl {

class SwapchainOutputPass;
class RendererBase;

inline class Layer_ : public Listener {

	friend class Rendering;

public:
	Layer_();
	~Layer_();

	void DrawFrame();

private:
	InFlightResources<vk::UniqueFence> m_frameFence;
	InFlightResources<vk::UniqueSemaphore> m_renderFinishedSem;
	InFlightResources<vk::UniqueSemaphore> m_imageAvailSem;

	InFlightCmdBuffers<Graphics> m_cmdBuffer;

	uint32 m_currentFrame{ 0 };

	SwapchainOutputPass* m_swapOutput{ nullptr };
	Scene* m_mainScene{ nullptr };
	RendererBase* m_renderer{ nullptr };
	RendererBase* m_currentRasterizer{ nullptr };

} * Layer{};
} // namespace vl
