#pragma once
#include "engine/Listener.h"
#include "rendering/wrappers/CmdBuffer.h"

struct Scene;

namespace vl {

class SwapchainOutputPass;

inline class Layer_ : public Listener {

	InFlightResources<vk::UniqueFence> m_frameFence;
	InFlightResources<vk::UniqueSemaphore> m_renderFinishedSem;
	InFlightResources<vk::UniqueSemaphore> m_imageAvailSem;

	InFlightCmdBuffers<Graphics> m_cmdBuffer;

	uint32 m_currentFrame{ 0 };

public:
	Layer_();
	~Layer_();

	void DrawFrame();


	SwapchainOutputPass* swapOutput;

	Scene* mainScene{ nullptr };
	Scene* currentScene{ nullptr };

} * Layer{};
} // namespace vl
