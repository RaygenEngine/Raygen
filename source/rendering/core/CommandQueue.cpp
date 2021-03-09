#include "pch.h"

#include "CommandQueue.h"

#include "Device.h"

CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE type)
	: m_type(type)
	, m_fenceValue(0)
{
	D3D12_COMMAND_QUEUE_DESC desc{};
	desc.Type = m_type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	auto d3d12Device = Device->GetHandle();

	AbortIfFailed(d3d12Device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_d3d12CommandQueue)));
	AbortIfFailed(d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_d3d12Fence)));
}

WRL::ComPtr<ID3D12GraphicsCommandList2> CommandQueue::GetCommandList()
{
	WRL::ComPtr<ID3D12CommandAllocator> d3d12CommandAllocator;
	WRL::ComPtr<ID3D12GraphicsCommandList2> d3d12CommandList;

	// find available or spawn cmd allocator
	if (!m_commandAllocatorQueue.empty() && IsFenceComplete(m_commandAllocatorQueue.front().fenceValue)) {
		d3d12CommandAllocator = m_commandAllocatorQueue.front().d3d12CommandAllocator;
		m_commandAllocatorQueue.pop();
	}
	else {
		d3d12CommandAllocator = Device->CreateCommandAllocator(m_type);
	}

	// find available or spawn
	if (!m_commandListQueue.empty()) {
		d3d12CommandList = m_commandListQueue.front();
		m_commandListQueue.pop();
	}
	else {
		d3d12CommandList = Device->CreateCommandList(d3d12CommandAllocator.Get(), m_type);
	}

	AbortIfFailed(d3d12CommandAllocator->Reset()); // CHECK:
	AbortIfFailed(d3d12CommandList->Reset(d3d12CommandAllocator.Get(), nullptr));

	// Associate the command allocator with the command list so that it can be
	// retrieved when the command list is executed.
	AbortIfFailed(
		d3d12CommandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), d3d12CommandAllocator.Get()));

	return d3d12CommandList;
}

uint64 CommandQueue::ExecuteCommandList(WRL::ComPtr<ID3D12GraphicsCommandList2> d3d12CommandList)
{
	d3d12CommandList->Close();

	ID3D12CommandAllocator* d3d12CommandAllocator;
	UINT dataSize = sizeof(d3d12CommandAllocator);
	AbortIfFailed(
		d3d12CommandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &d3d12CommandAllocator));

	ID3D12CommandList* const ppCommandLists[] = { d3d12CommandList.Get() };

	m_d3d12CommandQueue->ExecuteCommandLists(1, ppCommandLists);
	uint64 fenceValue = Signal();

	// put this list and allocator back as available
	m_commandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, d3d12CommandAllocator });
	m_commandListQueue.push(d3d12CommandList);

	// The ownership of the command allocator has been transferred to the ComPtr
	// in the command allocator queue. It is safe to release the reference
	// in this temporary COM pointer here.
	d3d12CommandAllocator->Release();

	return fenceValue;
}

uint64 CommandQueue::Signal()
{
	AbortIfFailed(m_d3d12CommandQueue->Signal(m_d3d12Fence.Get(), ++m_fenceValue));
	return m_fenceValue;
}

bool CommandQueue::IsFenceComplete(uint64 fenceValue)
{
	return m_d3d12Fence->GetCompletedValue() >= fenceValue;
}

void CommandQueue::WaitForFenceValue(uint64 fenceValue)
{
	if (!IsFenceComplete(fenceValue)) {
		auto fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

		AbortIfFailed(m_d3d12Fence->SetEventOnCompletion(fenceValue, fenceEvent));
		::WaitForSingleObject(fenceEvent, DWORD_MAX);

		::CloseHandle(fenceEvent);
	}
}

void CommandQueue::Flush()
{
	WaitForFenceValue(Signal());
}
