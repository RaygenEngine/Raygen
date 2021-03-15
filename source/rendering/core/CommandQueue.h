#pragma once

#include <queue>

class CommandQueue {

public:
	CommandQueue(D3D12_COMMAND_LIST_TYPE type);

	WRL::ComPtr<ID3D12GraphicsCommandList2> GetCommandList();

	// Execute a command list.
	// Returns the fence value to wait for for this command list.
	uint64 ExecuteCommandList(WRL::ComPtr<ID3D12GraphicsCommandList2> d3d12CommandList);
	void ExecuteCommandListBlocking(WRL::ComPtr<ID3D12GraphicsCommandList2> d3d12CommandList);

	// Synchronization objects
	uint64 Signal();
	bool IsFenceComplete(uint64 fenceValue);
	void WaitForFenceValue(uint64 fenceValue);
	void Flush();

	ID3D12CommandQueue* GetHandle() const { return m_d3d12CommandQueue.Get(); }

private:
	// Keep track of command allocators that are "in-flight"
	struct CommandAllocatorEntry {
		uint64 fenceValue;
		WRL::ComPtr<ID3D12CommandAllocator> d3d12CommandAllocator;
	};

	using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
	using CommandListQueue = std::queue<WRL::ComPtr<ID3D12GraphicsCommandList2>>;

	D3D12_COMMAND_LIST_TYPE m_type;
	WRL::ComPtr<ID3D12CommandQueue> m_d3d12CommandQueue;
	WRL::ComPtr<ID3D12Fence> m_d3d12Fence;
	uint64 m_fenceValue;

	CommandAllocatorQueue m_commandAllocatorQueue;
	CommandListQueue m_commandListQueue;
};
