#pragma once

class SwapChain {

public:
	SwapChain(bool vSync);

	// Number of swapchain back buffers.
	static const UINT s_bufferCount = 3;

	bool IsVSync() const { return m_vSync; }
	void SetVSync(bool vSync) { m_vSync = vSync; }
	void ToggleVSync() { SetVSync(!m_vSync); }


	UINT Present();

	void Resize(int32 width, int32 height);

	UINT GetCurrentBackBufferIndex() const { return m_currentBackBufferIndex; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView() const;
	WRL::ComPtr<ID3D12Resource> GetCurrentBackBuffer() const;

	glm::ivec2 GetSize() const { return { m_width, m_height }; }

protected:
	// Create the swap chain.
	WRL::ComPtr<IDXGISwapChain4> CreateSwapChain();

	// Update the render target views for the swapchain back buffers.
	void UpdateRTV();

private:
	bool m_vSync;
	bool m_isTearingSupported;

	WRL::ComPtr<IDXGISwapChain4> m_dxgiSwapChain;
	// texture resources (pointers, those exist inside the swap chain)
	WRL::ComPtr<ID3D12Resource> m_backBuffers[s_bufferCount];
	// render target views for the swap chain back buffers.
	WRL::ComPtr<ID3D12DescriptorHeap> m_d3d12RTVDescriptorHeap;

	int32 m_width{}, m_height{};

	UINT m_currentBackBufferIndex;
};
