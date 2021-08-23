#pragma once
#include "engine/Listener.h"
#include "rendering/wrappers/CmdBuffer.h"

struct Scene;

namespace vl {

class SwapchainOutputPass;
class RendererBase;

inline class Layer_ : public Listener {

public:
	Layer_();
	~Layer_();

	void DrawFrame();

	// TODO:
	SwapchainOutputPass* swapOutput{ nullptr };
	Scene* mainScene{ nullptr };
	RendererBase* renderer{ nullptr };

	void ResetMainScene();

	struct {
		BoolFlag flag;
		RendererBase* prev{ nullptr };
	} swapRenPath;

private:
	InFlightResources<vk::UniqueFence> m_frameFence;
	InFlightResources<vk::UniqueSemaphore> m_renderFinishedSem;
	InFlightResources<vk::UniqueSemaphore> m_imageAvailSem;

	InFlightCmdBuffers<Graphics> m_cmdBuffer;

	uint32 m_currentFrame{ 0 };

} * Layer{};
} // namespace vl
