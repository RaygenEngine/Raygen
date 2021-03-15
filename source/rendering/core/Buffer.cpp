#include "Buffer.h"

#include "Device.h"
#include "CommandQueue.h"

namespace {

void UpdateBufferResource(WRL::ComPtr<ID3D12GraphicsCommandList2> commandList, ID3D12Resource** pDestinationResource,
	ID3D12Resource** pIntermediateResource, size_t bufferSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags)
{
	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);

	// Create a committed resource for the GPU resource in a default heap.
	AbortIfFailed(Device->GetHandle()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(pDestinationResource)));

	// Create a committed resource for the upload.
	if (bufferData) {

		heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		resDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

		AbortIfFailed(Device->GetHandle()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(pIntermediateResource)));

		D3D12_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pData = bufferData;
		subresourceData.RowPitch = bufferSize;
		subresourceData.SlicePitch = subresourceData.RowPitch;

		UpdateSubresources(commandList.Get(), *pDestinationResource, *pIntermediateResource, 0, 0, 1, &subresourceData);
	}
}

} // namespace

Buffer::Buffer(size_t numElements, size_t elementSize)
	: bufferSize(numElements * elementSize)
{
}

void Buffer::UploadData(const void* data, size_t size, size_t offset)
{
	D3D12_RESOURCE_FLAGS flags{};

	CommandQueue& commandQueue
		= DeviceQueues[D3D12_COMMAND_LIST_TYPE_COPY]; // NEXT: what is going one with scopes and transitions here
	auto commandList = commandQueue.GetCommandList();


	// Upload vertex buffer data.
	WRL::ComPtr<ID3D12Resource> intermediateResource;
	UpdateBufferResource(commandList, &d3d12Resource, &intermediateResource, bufferSize, data, flags);

	commandQueue.ExecuteCommandListBlocking(commandList);
}

void Buffer::FillData(const void* data)
{
	UploadData(data, bufferSize);
}

VertexBuffer::VertexBuffer(size_t numElements, size_t elementSize)
	: Buffer(numElements, elementSize)
	, numVertices(numElements)
	, vertexStride(elementSize)
{
}

D3D12_VERTEX_BUFFER_VIEW VertexBuffer::GetBufferView() const
{
	D3D12_VERTEX_BUFFER_VIEW bv{};

	bv.BufferLocation = d3d12Resource->GetGPUVirtualAddress();
	bv.SizeInBytes = static_cast<UINT>(numVertices * vertexStride);
	bv.StrideInBytes = static_cast<UINT>(vertexStride);

	return bv;
}

IndexBuffer::IndexBuffer(size_t numElements, size_t elementSize)
	: Buffer(numElements, elementSize)
{
	assert(elementSize == 2 || elementSize == 4 && "Indices must be 16, or 32-bit integers.");

	numIndicies = numElements;
	indexFormat = (elementSize == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
}

D3D12_INDEX_BUFFER_VIEW IndexBuffer::GetBufferView() const
{
	uint indexSize = indexFormat == DXGI_FORMAT_R16_UINT ? 2u : 4u;

	D3D12_INDEX_BUFFER_VIEW bv{};

	bv.BufferLocation = d3d12Resource->GetGPUVirtualAddress();
	bv.SizeInBytes = indexSize * numIndicies;
	bv.Format = indexFormat;

	return bv;
}
