#include "Buffer.h"

#include "rendering/Device.h"
#include "rendering/wrappers/CmdBuffer.h"

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

	uHandle = Device->createBufferUnique(bufferInfo);

	vk::MemoryRequirements memRequirements = Device->getBufferMemoryRequirements(handle());

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
	uMemory = Device->allocateMemoryUnique(allocInfo);

	Device->bindBufferMemory(uHandle.get(), uMemory.get(), 0);


	const bool needsDeviceAddr = (allocFlags & vk::MemoryAllocateFlagBits::eDeviceAddress).operator bool();

	if (needsDeviceAddr) {
		deviceAddress = Device->getBufferAddress(handle());
	}
}

void RBuffer::CopyBuffer(const RBuffer& other)
{
	vk::BufferCopy copyRegion{};
	copyRegion.setSize(other.size);
	CopyBufferWithRegion(other, copyRegion);
}

void RBuffer::CopyBufferWithRegion(const RBuffer& other, vk::BufferCopy copyRegion)
{
	// WIP:
	ScopedOneTimeSubmitCmdBuffer<Dma> cmdBuffer{};
	cmdBuffer.copyBuffer(other.handle(), uHandle.get(), copyRegion);
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
		void* dptr = Device->mapMemory(uMemory.get(), offset, uploadSize);
		memcpy(dptr, data, uploadSize);
		Device->unmapMemory(uMemory.get());
	}
}

void RBuffer::UploadData(const std::vector<byte>& data)
{
	UploadData(data.data(), data.size());
}

RBuffer RBuffer::CreateTransfer(const char* name, MemorySpan memory, vk::BufferUsageFlags usageFlags)
{
	vk::DeviceSize bufferSize = memory.size();

	vl::RBuffer stagingbuffer{ bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	DEBUG_NAME(stagingbuffer, "[StagingBuffer]" + std::string(name));


	// copy data to buffer
	stagingbuffer.UploadData(memory.data(), memory.size());


	// device local
	auto buffer = vl::RBuffer{ memory.size(), vk::BufferUsageFlagBits::eTransferDst | usageFlags,
		vk::MemoryPropertyFlagBits::eDeviceLocal };

	DEBUG_NAME(buffer, name);

	// copy from host to device local
	buffer.CopyBuffer(stagingbuffer);

	return buffer;
}

} // namespace vl
