#pragma once

struct Scene;

namespace vl {

struct RSwapchain;

inline class Layer_ {

	BoolFlag m_didViewportResize;


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
