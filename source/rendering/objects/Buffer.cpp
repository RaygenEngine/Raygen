#include "pch.h"
#include "Buffer.h"

#include "rendering/Device.h"

namespace vl {
RawBuffer::RawBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
	: m_size(size)
{
	vk::BufferCreateInfo bufferInfo{};
	bufferInfo.setSize(size).setUsage(usage).setSharingMode(vk::SharingMode::eExclusive);

	m_handle = Device->createBufferUnique(bufferInfo);
	vk::MemoryRequirements memRequirements = Device->getBufferMemoryRequirements(m_handle.get());

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.setAllocationSize(memRequirements.size);
	allocInfo.setMemoryTypeIndex(Device->pd->FindMemoryType(memRequirements.memoryTypeBits, properties));

	// From https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer
	// It should be noted that in a real world application, you're not supposed to actually call vkAllocateMemory
	// for every individual buffer. The maximum number of simultaneous memory allocations is limited by the
	// maxMemoryAllocationCount physical device limit, which may be as low as 4096 even on high end hardware like an
	// NVIDIA GTX 1080. The right way to allocate memory for a large number of objects at the same time is to create
	// a custom allocator that splits up a single allocation among many different objects by using the offset
	// parameters that we've seen in many functions.
	m_memory = Device->allocateMemoryUnique(allocInfo);

	Device->bindBufferMemory(m_handle.get(), m_memory.get(), 0);
}

void RawBuffer::CopyBuffer(const RawBuffer& other)
{
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	Device->transferCmdBuffer.begin(beginInfo);

	vk::BufferCopy copyRegion{};
	copyRegion.setSize(other.m_size);

	Device->transferCmdBuffer.copyBuffer(other, m_handle.get(), 1, &copyRegion);

	Device->transferCmdBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBufferCount(1u);
	submitInfo.setPCommandBuffers(&Device->transferCmdBuffer);

	Device->transferQueue.submit(1u, &submitInfo, {});
	// PERF:
	// A fence would allow you to schedule multiple transfers simultaneously and wait for all of them complete,
	// instead of executing one at a time. That may give the driver more opportunities to optimize.
	Device->transferQueue.waitIdle();
}

void RawBuffer::UploadData(const void* data, size_t size)
{
	void* dptr = Device->mapMemory(m_memory.get(), 0, size);
	memcpy(dptr, data, size);
	Device->unmapMemory(m_memory.get());
}
} // namespace vl
