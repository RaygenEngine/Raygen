#pragma once

struct Scene;

namespace vl {

struct RSwapchain;

inline class Layer_ {

	BoolFlag m_didViewportResize;

	FrameArray<vk::UniqueFence> m_inFlightFence;
	FrameArray<vk::UniqueSemaphore> m_renderFinishedSem;
	FrameArray<vk::UniqueSemaphore> m_imageAvailSem;


	FrameArray<vk::CommandBuffer> m_cmdBuffer;


	inline static uint32 currentFrame{ 0 };



public:
	Layer_();
	~Layer_();

	void DrawFrame();

	UniquePtr<RSwapchain> mainSwapchain;
	UniquePtr<RSwapchain> secondSwapchain;
	UniquePtr<Scene> mainScene;
	UniquePtr<Scene> secondScene;

	Scene* currentScene{ nullptr };
} * Layer{};
} // namespace vl
