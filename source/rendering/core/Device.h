#pragma once

class CommandQueue;

inline struct DeviceQueues {
	std::array<UniquePtr<CommandQueue>, 7> queues;

	size_t GetIndexFromType(D3D12_COMMAND_LIST_TYPE type) const
	{
		switch (type) {
			case D3D12_COMMAND_LIST_TYPE_DIRECT: return 0;
			case D3D12_COMMAND_LIST_TYPE_BUNDLE: return 1;
			case D3D12_COMMAND_LIST_TYPE_COMPUTE: return 2;
			case D3D12_COMMAND_LIST_TYPE_COPY: return 3;
			case D3D12_COMMAND_LIST_TYPE_VIDEO_DECODE: return 4;
			case D3D12_COMMAND_LIST_TYPE_VIDEO_PROCESS: return 5;
			case D3D12_COMMAND_LIST_TYPE_VIDEO_ENCODE: return 6;
			default: return -1;
		}
	}

	void AddQueue(D3D12_COMMAND_LIST_TYPE type);
	CommandQueue& operator[](D3D12_COMMAND_LIST_TYPE type) const { return *queues[GetIndexFromType(type)]; }

	// flush all queues
	void FlushAll();
} DeviceQueues;

inline class Device_ {

public:
	Device_();


	WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 numDescriptors);
	WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type);
	WRL::ComPtr<ID3D12GraphicsCommandList2> CreateCommandList(
		ID3D12CommandAllocator* d3d12CommandAllocator, D3D12_COMMAND_LIST_TYPE type);

	ID3D12Device2* GetHandle() const { return m_d3d12Device.Get(); }

	void Flush() const { DeviceQueues.FlushAll(); }

	UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE index) const
	{
		return m_descriptorHandleIncrementSize[index];
	}

	// NEXT: remove
	void UpdateBufferResource(WRL::ComPtr<ID3D12GraphicsCommandList2> cmdList, ID3D12Resource** pDestinationResource,
		ID3D12Resource** pIntermediateResource, size_t numElements, size_t elementSize, const void* bufferData,
		D3D12_RESOURCE_FLAGS flags = {})
	{
		size_t bufferSize = numElements * elementSize;

		auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

		// Create a committed resource for the GPU resource in a default heap.
		AbortIfFailed(m_d3d12Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(pDestinationResource)));

		// Create a committed resource for the upload.
		if (bufferData) {
			auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

			AbortIfFailed(m_d3d12Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(pIntermediateResource)));

			D3D12_SUBRESOURCE_DATA subresourceData = {};
			subresourceData.pData = bufferData;
			subresourceData.RowPitch = bufferSize;
			subresourceData.SlicePitch = subresourceData.RowPitch;

			UpdateSubresources(cmdList.Get(), *pDestinationResource, *pIntermediateResource, 0, 0, 1, &subresourceData);
		}
	}

protected:
	WRL::ComPtr<IDXGIAdapter4> GetAdapter(bool bUseWarp);
	WRL::ComPtr<ID3D12Device2> CreateDevice(WRL::ComPtr<IDXGIAdapter4> adapter);

private:
	std::array<UINT, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> m_descriptorHandleIncrementSize;
	WRL::ComPtr<ID3D12Device2> m_d3d12Device;
} * Device{ nullptr };
