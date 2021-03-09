#include "pch.h"

#include "SwapChain.h"

#include "platform/Platform.h"
#include "rendering/Layer.h"
#include "rendering/core/Device.h"
#include "rendering/core/CommandQueue.h"

namespace {
bool CheckTearingSupport()
{
	BOOL allowTearing = FALSE;

	// Rather than create the DXGI 1.5 factory interface directly, we create the
	// DXGI 1.4 interface and query for the 1.5 interface. This is to enable the
	// graphics debugging tools which will not support the 1.5 factory interface
	// until a future update.
	WRL::ComPtr<IDXGIFactory4> factory4;
	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4)))) {
		WRL::ComPtr<IDXGIFactory5> factory5;
		if (SUCCEEDED(factory4.As(&factory5))) {
			factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
		}
	}

	return allowTearing == TRUE;
}
} // namespace


SwapChain::SwapChain(bool vSync)
	: m_vSync(vSync)
	, m_isTearingSupported(CheckTearingSupport())
{
	auto d3d12Device = Device->GetHandle();

	m_dxgiSwapChain = CreateSwapChain();
	m_d3d12RTVDescriptorHeap = Device->CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, s_bufferCount);

	UpdateRTV();
}

UINT SwapChain::Present()
{
	UINT syncInterval = m_vSync ? 1 : 0;
	UINT presentFlags = m_isTearingSupported && !m_vSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
	AbortIfFailed(m_dxgiSwapChain->Present(syncInterval, presentFlags));

	m_currentBackBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();

	return m_currentBackBufferIndex;
}

void SwapChain::Resize(int32 width, int32 height)
{
	m_width = width;
	m_height = height;

	for (int i = 0; i < s_bufferCount; ++i) {
		// Any references to the back buffers must be released
		// before the swap chain can be resized.
		m_backBuffers[i].Reset();
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	AbortIfFailed(m_dxgiSwapChain->GetDesc(&swapChainDesc));
	AbortIfFailed(m_dxgiSwapChain->ResizeBuffers(
		s_bufferCount, width, height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

	m_currentBackBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();

	UpdateRTV();
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::GetCurrentRenderTargetView() const
{
	auto rtvDescriptorSize = Device->GetHandle()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_d3d12RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), GetCurrentBackBufferIndex(), rtvDescriptorSize);
}

WRL::ComPtr<ID3D12Resource> SwapChain::GetCurrentBackBuffer() const
{
	return m_backBuffers[GetCurrentBackBufferIndex()];
}

WRL::ComPtr<IDXGISwapChain4> SwapChain::CreateSwapChain()
{
	WRL::ComPtr<IDXGISwapChain4> dxgiSwapChain4;
	WRL::ComPtr<IDXGIFactory4> dxgiFactory4;
	UINT createFactoryFlags = 0;
#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	AbortIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

	auto windowSize = Platform::GetMainSize();
	m_width = windowSize.x;
	m_height = windowSize.y;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc = { 1, 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = s_bufferCount;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	// It is recommended to always allow tearing if tearing support is available.
	swapChainDesc.Flags = m_isTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
	ID3D12CommandQueue* pCommandQueue = DeviceQueues[D3D12_COMMAND_LIST_TYPE_DIRECT].GetHandle();

	HWND hwnd = Platform::GetMainNativeHandle();

	WRL::ComPtr<IDXGISwapChain1> swapChain1;
	AbortIfFailed(
		dxgiFactory4->CreateSwapChainForHwnd(pCommandQueue, hwnd, &swapChainDesc, nullptr, nullptr, &swapChain1));

	// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
	// will be handled manually.
	// AbortIfFailed(dxgiFactory4->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

	AbortIfFailed(swapChain1.As(&dxgiSwapChain4));

	m_currentBackBufferIndex = dxgiSwapChain4->GetCurrentBackBufferIndex();

	return dxgiSwapChain4;
}

void SwapChain::UpdateRTV()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_d3d12RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto d3d12Device = Device->GetHandle();
	auto rtvDescriptorSize
		= d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); // WIP: cache if costly

	for (int i = 0; i < s_bufferCount; ++i) {
		WRL::ComPtr<ID3D12Resource> d3d12Resource;
		AbortIfFailed(m_dxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&d3d12Resource)));

		d3d12Device->CreateRenderTargetView(d3d12Resource.Get(), nullptr, rtvHandle);

		m_backBuffers[i] = d3d12Resource;

		rtvHandle.Offset(rtvDescriptorSize);
	}
}
