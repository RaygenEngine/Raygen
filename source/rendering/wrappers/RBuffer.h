#pragma once
#include <vulkan/vulkan.hpp>

namespace vl {

class RBuffer {
	vk::DeviceSize m_size;

	vk::UniqueBuffer m_handle;
	vk::UniqueDeviceMemory m_memory;

public:
	RBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);


	void CopyBuffer(const RBuffer& other);
	void UploadData(const void* data, size_t size);
	void UploadData(const std::vector<byte>& data);

	template<typename T>
	void UploadDataTemplate(const T& data)
	{
		UploadData(&data, sizeof(T));
	}

	operator vk::Buffer() const noexcept { return m_handle.get(); }
	vk::DeviceMemory GetMemory() const { return m_memory.get(); }

	[[nodiscard]] vk::DeviceSize GetSize() const noexcept { return m_size; }
};

// template<typename T>
// class RUboBuffer : public RBuffer {
//
// public:
//	RUboBuffer(vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
//		: RBuffer(sizeof(T), usage, properties)
//	{
//	}
//
//	void UploadData(const T& data) { RBuffer::UploadData(&data, sizeof(T)); }
//};
} // namespace vl
