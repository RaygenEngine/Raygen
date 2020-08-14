#pragma once

namespace vl {

struct RBuffer {

	vk::DeviceSize size{};

	RBuffer() = default;
	RBuffer(vk::DeviceSize inSize, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
		vk::MemoryAllocateFlags allocFlags = {});


	void CopyBuffer(const RBuffer& other);
	void UploadData(const void* data, size_t uploadSize);
	void UploadData(const std::vector<byte>& data);

	template<typename T>
	void UploadDataTemplate(const T& data)
	{
		UploadData(&data, sizeof(T));
	}

	operator vk::Buffer() const { return handle.get(); }
	vk::DeviceMemory GetMemory() const { return memory.get(); }

	[[nodiscard]] vk::DeviceAddress GetAddress() const noexcept;

private:
	vk::UniqueBuffer handle;
	vk::UniqueDeviceMemory memory;
};
} // namespace vl
