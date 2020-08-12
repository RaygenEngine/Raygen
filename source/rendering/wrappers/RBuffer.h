#pragma once

namespace vl {

struct RBuffer {

	vk::DeviceSize size{};

	RBuffer() = default;
	RBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
		vk::MemoryAllocateFlags allocFlags = {});


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

	[[nodiscard]] vk::DeviceAddress GetAddress() const noexcept;

private:
	vk::UniqueBuffer m_handle;
	vk::UniqueDeviceMemory m_memory;
};
} // namespace vl
