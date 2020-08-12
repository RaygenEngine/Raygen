#pragma once
#include "engine/Listener.h"

struct Scene;

namespace vl {

struct RSwapchain;

inline class Layer_ : public Listener {

	BoolFlag m_didViewportResize;

	FrameArray<vk::UniqueFence> m_inFlightFence;
	FrameArray<vk::UniqueSemaphore> m_renderFinishedSem;
	FrameArray<vk::UniqueSemaphore> m_imageAvailSem;


	FrameArray<vk::CommandBuffer> m_cmdBuffer;


	uint32 currentFrame{ 0 };


	BoolFlag m_didWindowResize;
	bool m_isMinimized{ false };


public:
	Layer_();
	~Layer_();

	void DrawFrame();

	UniquePtr<RSwapchain> mainSwapchain;
	UniquePtr<RSwapchain> secondSwapchain;
	UniquePtr<Scene> mainScene;


	Scene* currentScene{ nullptr };
} * Layer{};
} // namespace vl
