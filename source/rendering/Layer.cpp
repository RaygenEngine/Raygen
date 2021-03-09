#include "pch.h"

#include "Layer.h"

#include "engine/Events.h"
#include "engine/Engine.h"
#include "core/Device.h"
#include "core/CommandQueue.h"
#include "core/SwapChain.h"
#include "editor/imgui/ImguiImpl.h"
#include "engine/console/ConsoleVariable.h"

Layer_::Layer_()
{
	Device = new Device_();
	DeviceQueues.AddQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

	swapChain = std::make_unique<SwapChain>(false);

	Event::OnWindowResize.Bind(this, [&swapref = *swapChain](int32 width, int32 height) {
		Device->Flush();
		swapref.Resize(width, height);
	});
}

Layer_::~Layer_()
{
	Device->Flush();

	delete Device;
}


void Layer_::DrawFrame()
{
	CommandQueue& dqueue = DeviceQueues[D3D12_COMMAND_LIST_TYPE_DIRECT];
	auto d3d12CommandList = dqueue.GetCommandList();

	UINT currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();
	auto backBuffer = swapChain->GetCurrentBackBuffer();
	auto rtv = swapChain->GetCurrentRenderTargetView();

	// Clear the render targets.
	{
		TransitionResource(
			d3d12CommandList, backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		FLOAT clearColor[]{ 0.15f, 0.15f, 0.1f, 1.0f };
		d3d12CommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	}

	d3d12CommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

	ImguiImpl::RenderDX12(d3d12CommandList.Get());

	// Present
	{
		TransitionResource(
			d3d12CommandList, backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		frameFenceValues[currentBackBufferIndex] = dqueue.ExecuteCommandList(d3d12CommandList);

		currentBackBufferIndex = swapChain->Present();

		dqueue.WaitForFenceValue(frameFenceValues[currentBackBufferIndex]);
	}

	++m_currFrame;
}
