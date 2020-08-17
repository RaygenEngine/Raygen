#pragma once

// WIP: Specialize register debug name without this?
#include "rendering/Device.h"

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

	operator vk::Buffer() const { return handle.get(); }
	vk::DeviceMemory GetMemory() const { return memory.get(); }

	[[nodiscard]] vk::DeviceAddress GetAddress() const noexcept;

	vk::UniqueBuffer handle;
	vk::UniqueDeviceMemory memory;
};


} // namespace vl


namespace detail {
template<>
inline void RegisterDebugName<vl::RBuffer>(const vl::RBuffer& buffer, const std::string& name)
{
	vk::DebugUtilsObjectNameInfoEXT debugNameInfo{};

	detail::RegisterDebugName(buffer.handle.get(), name + " [buffer]");
	detail::RegisterDebugName(buffer.memory.get(), name + " [memory]");
}
} // namespace detail
