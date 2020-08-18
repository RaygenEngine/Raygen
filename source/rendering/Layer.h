#pragma once
#include "engine/Listener.h"

struct Scene;

namespace vl {

struct RSwapchain;

inline class Layer_ : public Listener {

	BoolFlag m_didViewportResize;

	InFlightResource<vk::UniqueFence> m_fences;
	InFlightResource<vk::UniqueSemaphore> m_renderFinishedSems;
	InFlightResource<vk::UniqueSemaphore> m_imageAvailSems;

	InFlightResource<vk::CommandBuffer> m_cmdBuffers;

	uint32 currentFrame{ 0 };

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
