#pragma once
#include "engine/Listener.h"

struct Scene;

namespace vl {

struct RSwapchain;

inline class Layer_ : public Listener {

	BoolFlag m_didViewportResize;

	InFlightResources<vk::UniqueFence> m_frameFence;
	InFlightResources<vk::UniqueSemaphore> m_renderFinishedSem;
	InFlightResources<vk::UniqueSemaphore> m_imageAvailSem;

	InFlightResources<vk::CommandBuffer> m_cmdBuffer;

	uint32 m_currentFrame{ 0 };

	BoolFlag m_didWindowResize;
	bool m_isMinimized{ false };


public:
	Layer_();
	~Layer_();

	void DrawFrame();

	RSwapchain* mainSwapchain;
	Scene* mainScene;

	Scene* currentScene{ nullptr };
} * Layer{};
} // namespace vl
