#pragma once
#include "engine/Listener.h"

class SwapChain;

constexpr size_t c_inFlightFrames{ 3 };

inline class Layer_ : public Listener {

public:
	Layer_();
	~Layer_();

	void DrawFrame();

	uint32 GetCurrentFrame() const { return m_currFrame; }


private:
	uint32 m_currFrame{ 0 };
	uint64 frameFenceValues[c_inFlightFrames]{};

	UniquePtr<SwapChain> swapChain;


} * Layer{};
