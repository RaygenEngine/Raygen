#pragma once
#include "engine/Listener.h"

class SwapChain;
struct Scene;

constexpr size_t c_inFlightFrames{ 3 };

inline class Layer_ : public Listener {

public:
	Layer_();
	~Layer_();

	void DrawFrame();

	uint32 GetCurrentFrame() const { return m_currFrame; }


	Scene* GetScene() const { return m_scene.get(); }

private:
	uint32 m_currFrame{ 0 };
	uint64 m_frameFenceValues[c_inFlightFrames]{};

	UniquePtr<SwapChain> m_swapChain;
	UniquePtr<Scene> m_scene;

	friend class ImguiImpl;


} * Layer{};
