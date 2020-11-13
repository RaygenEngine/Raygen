#pragma once

namespace vl {

struct RBuffer {

	vk::DeviceSize size{};

	RBuffer() = default;
	RBuffer(vk::DeviceSize inSize, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
		vk::MemoryAllocateFlags allocFlags = {});

	void CopyBuffer(const RBuffer& other);
	void CopyBufferWithRegion(const RBuffer& other, vk::BufferCopy copyRegion);
	size_t CopyBufferAt(const RBuffer& other, size_t offset, size_t copySize = 0);

	void UploadData(const void* data, size_t uploadSize, size_t offset = 0);
	void UploadData(const std::vector<byte>& data);

	template<typename T>
	void UploadDataTemplate(const T& data)
	{
		UploadData(&data, sizeof(T));
	}

	[[nodiscard]] vk::Buffer handle() const { return uHandle.get(); }
	[[nodiscard]] vk::DeviceMemory memory() const { return uMemory.get(); }

	[[nodiscard]] VkDeviceAddress address() const
	{
		CLOG_ERROR(
			deviceAddress == 0, "Attempting to get device address from buffer without proper Device Address flag");
		return deviceAddress;
	};


	// Creates a gpu buffer from given cpu memory using a staging buffer.
	static RBuffer CreateTransfer(const char* name, MemorySpan memory, vk::BufferUsageFlags usageFlags);


private:
	vk::UniqueBuffer uHandle;
	vk::UniqueDeviceMemory uMemory;
	VkDeviceAddress deviceAddress{ 0 };
};
} // namespace vl

//
// namespace detail {
// template<>
// inline void RegisterDebugName<vl::RBuffer>(const vl::RBuffer& buffer, const std::string& name)
//{
//	vk::DebugUtilsObjectNameInfoEXT debugNameInfo{};
//
//	detail::RegisterDebugName(buffer.handle(), name + " [buffer]");
//	detail::RegisterDebugName(buffer.memory(), name + " [memory]");
//}
//} // namespace detail
