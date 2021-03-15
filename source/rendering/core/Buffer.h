#pragma once

struct Buffer {

	Buffer(size_t numElements, size_t elementSize);
	~Buffer() { LOG_REPORT("dfasfsa"); }

	void UploadData(const void* data, size_t size, size_t offset = 0);
	void FillData(const void* data);

	size_t bufferSize;
	WRL::ComPtr<ID3D12Resource> d3d12Resource{};
};

struct VertexBuffer : public Buffer {

	VertexBuffer(size_t numElements, size_t elementSize);

	D3D12_VERTEX_BUFFER_VIEW GetBufferView() const;

	size_t numVertices;
	size_t vertexStride;
};


struct IndexBuffer : public Buffer {
	IndexBuffer(size_t numElements, size_t elementSize);

	D3D12_INDEX_BUFFER_VIEW GetBufferView() const;

	size_t numIndicies;
	DXGI_FORMAT indexFormat;
};
