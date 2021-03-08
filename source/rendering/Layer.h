#pragma once
#include "engine/Listener.h"

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

	BoolFlag swapRenderer;

private:
	uint32 m_currentFrame{ 0 };

} * Layer{};
} // namespace vl
