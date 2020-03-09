#pragma once
#include <vulkan/vulkan.hpp>

// CHECK: if useful there could be a templated buffer class with arguement of a struct type that describes the memory
// layout e.g Buffer<UBO_Globals>, m_size = sizeof(UBO_Globals), UploadData(const UBO_Globals& data) etc...
class Buffer {
	vk::DeviceSize m_size;

	vk::UniqueBuffer m_handle;
	vk::UniqueDeviceMemory m_memory;

public:
	Buffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

	void CopyBuffer(const Buffer& other);
	void UploadData(const void* data, size_t size);

	operator vk::Buffer() const noexcept { return m_handle.get(); }
};
