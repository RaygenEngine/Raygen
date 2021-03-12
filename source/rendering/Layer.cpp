#include "pch.h"

#include "Layer.h"

#include "engine/Events.h"
#include "engine/Engine.h"
#include "core/Device.h"
#include "core/CommandQueue.h"
#include "core/SwapChain.h"
#include "editor/imgui/ImguiImpl.h"
#include "engine/console/ConsoleVariable.h"
#include "rendering/scene/Scene.h"

Layer_::Layer_()
{
	Device = new Device_();
	DeviceQueues.AddQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

	m_swapChain = std::make_unique<SwapChain>(false);
	m_scene = std::make_unique<Scene>();

	Event::OnWindowResize.Bind(this, [&swapref = *m_swapChain](int32 width, int32 height) {
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
	m_scene->ConsumeCmdQueue();

	m_scene->UploadDirty(m_currFrame); // NEW:: param

	CommandQueue& dqueue = DeviceQueues[D3D12_COMMAND_LIST_TYPE_DIRECT];
	auto d3d12CommandList = dqueue.GetCommandList();

	UINT currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
	auto backBuffer = m_swapChain->GetCurrentBackBuffer();
	auto rtv = m_swapChain->GetCurrentRenderTargetView();

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

		m_frameFenceValues[currentBackBufferIndex] = dqueue.ExecuteCommandList(d3d12CommandList);

		currentBackBufferIndex = m_swapChain->Present();

		dqueue.WaitForFenceValue(m_frameFenceValues[currentBackBufferIndex]);
	}

	++m_currFrame;
}
