#include "CmdBuffer.h"

#include "rendering/Device.h"

namespace vl {

namespace impl {
	CmdBufferBase::CmdBufferBase(CmdPool& cpool, vk::CommandBuffer cmdBuffer)
		: pool(cpool)
	{
		vk::CommandBuffer::operator=(cmdBuffer);
	}

	CmdBufferBase::CmdBufferBase(CmdPool& cpool, vk::CommandBufferLevel level)
		: pool(cpool)
	{
		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo
			.setCommandPool(pool.handle()) //
			.setLevel(level)
			.setCommandBufferCount(1u);

		vk::CommandBuffer::operator=(Device->allocateCommandBuffers(allocInfo)[0]);
	}

	CmdBufferBase::~CmdBufferBase() { Device->freeCommandBuffers(pool.handle(), *this); }

	void CmdBufferBase::begin(const vk::CommandBufferBeginInfo& beginInfo) { vk::CommandBuffer::begin(beginInfo); }

	void CmdBufferBase::begin()
	{
		vk::CommandBufferBeginInfo beginInfo{};
		begin(beginInfo);
	}

	void CmdBufferBase::submit(vk::SubmitInfo& submitInfo, const vk::Fence& fence)
	{
		submitInfo.setCommandBuffers(*this);
		pool.queue.submit(submitInfo, fence);
	}

	void CmdBufferBase::submit()
	{
		vk::SubmitInfo submitInfo{};
		submitInfo.setCommandBuffers(*this);

		submit(submitInfo);
	}

	CmdBufferCollectionBase::CmdBufferCollectionBase(CmdPool& cpool, vk::CommandBufferLevel level, uint32 count)
	{
		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo
			.setCommandPool(cpool.handle()) //
			.setLevel(level)
			.setCommandBufferCount(count);

		auto handles = Device->allocateCommandBuffers(allocInfo);

		for (auto handle : handles) {
			uHandles.emplace_back(std::make_unique<CmdBufferBase>(cpool, handle));
		}
	}

	ScopedOneTimeSubmitCmdBufferImpl::ScopedOneTimeSubmitCmdBufferImpl(CmdPool& cpool)
		: CmdBufferBase(cpool, vk::CommandBufferLevel::ePrimary)
	{
		vk::CommandBufferBeginInfo beginInfo{};
		beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		begin(beginInfo);
	}

	ScopedOneTimeSubmitCmdBufferImpl::~ScopedOneTimeSubmitCmdBufferImpl()
	{
		end();

		submit();
		pool.queue.waitIdle(); // PERF:
		// A fence would allow you to schedule multiple transfers simultaneously and wait for all of them to complete,
		// instead of executing one at a time. That may give the driver more opportunities to optimize.
	}
} // namespace impl

CmdBuffer<Compute>::CmdBuffer(vk::CommandBufferLevel level)
	: CmdBufferBase(CmdPoolManager->computeCmdPool, level)
{
}

CmdBuffer<Graphics>::CmdBuffer(vk::CommandBufferLevel level)
	: CmdBufferBase(CmdPoolManager->graphicsCmdPool, level)
{
}

CmdBuffer<Dma>::CmdBuffer(vk::CommandBufferLevel level)
	: CmdBufferBase(CmdPoolManager->dmaCmdPool, level)
{
}

ScopedOneTimeSubmitCmdBuffer<Compute>::ScopedOneTimeSubmitCmdBuffer()
	: ScopedOneTimeSubmitCmdBufferImpl(CmdPoolManager->computeCmdPool)
{
}

ScopedOneTimeSubmitCmdBuffer<Graphics>::ScopedOneTimeSubmitCmdBuffer()
	: ScopedOneTimeSubmitCmdBufferImpl(CmdPoolManager->graphicsCmdPool)
{
}

ScopedOneTimeSubmitCmdBuffer<Dma>::ScopedOneTimeSubmitCmdBuffer()
	: ScopedOneTimeSubmitCmdBufferImpl(CmdPoolManager->dmaCmdPool)
{
}

CmdBuffers<Compute>::CmdBuffers(vk::CommandBufferLevel level, uint32 count)
	: CmdBufferCollectionBase(CmdPoolManager->computeCmdPool, level, count)
{
}

CmdBuffers<Graphics>::CmdBuffers(vk::CommandBufferLevel level, uint32 count)
	: CmdBufferCollectionBase(CmdPoolManager->graphicsCmdPool, level, count)
{
}

CmdBuffers<Dma>::CmdBuffers(vk::CommandBufferLevel level, uint32 count)
	: CmdBufferCollectionBase(CmdPoolManager->dmaCmdPool, level, count)
{
}

} // namespace vl
