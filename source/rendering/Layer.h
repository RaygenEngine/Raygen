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

protected:
	void DrawStaticGeometry(WRL::ComPtr<ID3D12GraphicsCommandList2>& commandList);
	void PreparePipelineStuff();

private:
	uint32 m_currFrame{ 0 };
	uint64 m_frameFenceValues[c_inFlightFrames]{};

	UniquePtr<SwapChain> m_swapChain;
	UniquePtr<Scene> m_scene;

	friend class ImguiImpl;


	// WIP: render targets
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;

	WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	WRL::ComPtr<ID3D12PipelineState> m_pipelineState;

	// Depth buffer.
	WRL::ComPtr<ID3D12Resource> depthBuffer;
	// Descriptor heap for depth buffer.
	WRL::ComPtr<ID3D12DescriptorHeap> DSVDescriptorHeap;

	void UpdateDSV(int32_t width, int32_t height);


} * Layer{};
