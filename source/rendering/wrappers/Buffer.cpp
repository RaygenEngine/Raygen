#include "pch.h"
#include "Buffer.h"

#include "rendering/Device.h"

namespace vl {


RBuffer::RBuffer(vk::DeviceSize inSize, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
	vk::MemoryAllocateFlags allocFlags)
	: size(
		std::max(inSize, 1llu)) // Fixes many allocation of 0 errors. Better if checks can usually workaround these
								// calls but in general we just do something that will not crash and spit out a warning
{
	CLOG_WARN(inSize == 0, "RBuffer requested with size 0 bytes. A buffer of size 1 byte will be given instead.");

	vk::BufferCreateInfo bufferInfo{};
	bufferInfo
		.setSize(size) //
		.setUsage(usage)
		.setSharingMode(vk::SharingMode::eExclusive);

	handle = Device->createBufferUnique(bufferInfo);

	vk::MemoryRequirements memRequirements = Device->getBufferMemoryRequirements(handle.get());

	vk::StructureChain<vk::MemoryAllocateInfo, vk::MemoryAllocateFlagsInfo> allocInfoChain{};
	auto& allocInfo = allocInfoChain.get<vk::MemoryAllocateInfo>();
	auto& allocFlagInfo = allocInfoChain.get<vk::MemoryAllocateFlagsInfo>();

	allocInfo
		.setAllocationSize(memRequirements.size) //
		.setMemoryTypeIndex(Device->FindMemoryType(memRequirements.memoryTypeBits, properties));
	allocFlagInfo.setFlags(allocFlags);

	// From https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer
	// It should be noted that in a real world application, you're not supposed to actually call vkAllocateMemory
	// for every individual buffer. The maximum number of simultaneous memory allocations is limited by the
	// maxMemoryAllocationCount physical device limit, which may be as low as 4096 even on high end hardware like an
	// NVIDIA GTX 1080. The right way to allocate memory for a large number of objects at the same time is to create
	// a custom allocator that splits up a single allocation among many different objects by using the offset
	// parameters that we've seen in many functions.
	memory = Device->allocateMemoryUnique(allocInfo);

	Device->bindBufferMemory(handle.get(), memory.get(), 0);
}

void RBuffer::CopyBuffer(const RBuffer& other)
{
	vk::BufferCopy copyRegion{};
	copyRegion.setSize(other.size);
	CopyBufferWithRegion(other, copyRegion);
}

void RBuffer::CopyBufferWithRegion(const RBuffer& other, vk::BufferCopy copyRegion)
{
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	Device->dmaCmdBuffer.begin(beginInfo);
	Device->dmaCmdBuffer.copyBuffer(other, handle.get(), 1, &copyRegion);
	Device->dmaCmdBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBufferCount(1u);
	submitInfo.setPCommandBuffers(&Device->dmaCmdBuffer);

	Device->dmaQueue.submit(1u, &submitInfo, {});
	// PERF:
	// A fence would allow you to schedule multiple transfers simultaneously and wait for all of them complete,
	// instead of executing one at a time. That may give the driver more opportunities to optimize.
	Device->dmaQueue.waitIdle();
}

size_t RBuffer::CopyBufferAt(const RBuffer& other, size_t offset, size_t copySize)
{
	copySize = copySize == 0 ? other.size : copySize;
	vk::BufferCopy copyRegion{};
	copyRegion
		.setSize(copySize) //
		.setDstOffset(offset);
	CopyBufferWithRegion(other, copyRegion);

	return offset + copySize;
}

void RBuffer::UploadData(const void* data, size_t uploadSize, size_t offset)
{
	CLOG_ERROR(uploadSize + offset > size, "Attempting to upload a bigger size to rbuffer of smaller size.");

	if (uploadSize > 0 && uploadSize + offset <= size) {
		void* dptr = Device->mapMemory(memory.get(), offset, uploadSize);
		memcpy(dptr, data, uploadSize);
		Device->unmapMemory(memory.get());
	}
}

void RBuffer::UploadData(const std::vector<byte>& data)
{
	UploadData(data.data(), data.size());
}

vk::DeviceAddress RBuffer::GetAddress() const noexcept
{
	return Device->getBufferAddress(vk::BufferDeviceAddressInfo{ handle.get() });
}


} // namespace vl
