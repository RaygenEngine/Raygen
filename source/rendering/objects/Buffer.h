#pragma once
#include <vulkan/vulkan.hpp>

namespace vl {
class RawBuffer {
	vk::DeviceSize m_size;

	vk::UniqueBuffer m_handle;
	vk::UniqueDeviceMemory m_memory;

public:
	RawBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

	void CopyBuffer(const RawBuffer& other);
	void UploadData(const void* data, size_t size);
	void UploadData(const std::vector<byte> data);

	operator vk::Buffer() const noexcept { return m_handle.get(); }
	vk::DeviceMemory GetMemory() const { return m_memory.get(); }
};

template<typename T>
class Buffer : public RawBuffer {

public:
	Buffer(vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
		: RawBuffer(sizeof(T), usage, properties)
	{
	}

	void UploadData(const T& data) { RawBuffer::UploadData(&data, sizeof(T)); }
};
} // namespace vl
