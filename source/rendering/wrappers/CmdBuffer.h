#pragma once

#include "rendering/wrappers/CmdPool.h"


namespace vl {

namespace impl {

	struct CmdBufferBase : public vk::CommandBuffer {

		CmdBufferBase(CmdBufferBase const&) = delete;
		CmdBufferBase(CmdBufferBase&&) = delete;
		CmdBufferBase& operator=(CmdBufferBase const&) = delete;
		CmdBufferBase& operator=(CmdBufferBase&&) = delete;

		CmdBufferBase(CmdPool& cpool, vk::CommandBuffer cmdBuffer);
		CmdBufferBase(CmdPool& cpool, vk::CommandBufferLevel level);
		virtual ~CmdBufferBase();

		void begin(const vk::CommandBufferBeginInfo& beginInfo);
		void begin();

		void submit(vk::SubmitInfo& submitInfo, const vk::Fence& fence = {});
		void submit();

	protected:
		CmdPool& pool;
	};

	struct CmdBufferCollectionBase {

		CmdBufferCollectionBase(CmdBufferCollectionBase const&) = delete;
		CmdBufferCollectionBase(CmdBufferCollectionBase&&) = default;
		CmdBufferCollectionBase& operator=(CmdBufferCollectionBase const&) = delete;
		CmdBufferCollectionBase& operator=(CmdBufferCollectionBase&&) = default;
		virtual ~CmdBufferCollectionBase() = default;

		CmdBufferCollectionBase() = default;
		CmdBufferCollectionBase(CmdPool& cpool, vk::CommandBufferLevel level, uint32 count);

		[[nodiscard]] CmdBufferBase& operator[](size_t index) const { return *uHandles.at(index); }

	protected:
		std::vector<UniquePtr<CmdBufferBase>> uHandles;
	};

	struct ScopedOneTimeSubmitCmdBufferImpl : public CmdBufferBase {

		ScopedOneTimeSubmitCmdBufferImpl(CmdPool& cpool);
		virtual ~ScopedOneTimeSubmitCmdBufferImpl();
	};

} // namespace impl

struct Compute;
struct Graphics;
struct Dma;

template<typename T>
struct CmdBuffer;

template<>
struct CmdBuffer<Compute> : public impl::CmdBufferBase {
	explicit CmdBuffer(vk::CommandBufferLevel level);
};

template<>
struct CmdBuffer<Graphics> : public impl::CmdBufferBase {
	explicit CmdBuffer(vk::CommandBufferLevel level);
};

template<>
struct CmdBuffer<Dma> : public impl::CmdBufferBase {
	explicit CmdBuffer(vk::CommandBufferLevel level);
};

template<typename T>
struct ScopedOneTimeSubmitCmdBuffer;

template<>
struct ScopedOneTimeSubmitCmdBuffer<Compute> : public impl::ScopedOneTimeSubmitCmdBufferImpl {
	explicit ScopedOneTimeSubmitCmdBuffer();
};

template<>
struct ScopedOneTimeSubmitCmdBuffer<Graphics> : public impl::ScopedOneTimeSubmitCmdBufferImpl {
	explicit ScopedOneTimeSubmitCmdBuffer();
};

template<>
struct ScopedOneTimeSubmitCmdBuffer<Dma> : public impl::ScopedOneTimeSubmitCmdBufferImpl {
	explicit ScopedOneTimeSubmitCmdBuffer();
};

template<typename T>
struct CmdBuffers;

template<>
struct CmdBuffers<Compute> : public impl::CmdBufferCollectionBase {
	CmdBuffers() = default;
	explicit CmdBuffers(vk::CommandBufferLevel level, uint32 count);
};

template<>
struct CmdBuffers<Graphics> : public impl::CmdBufferCollectionBase {
	CmdBuffers() = default;
	explicit CmdBuffers(vk::CommandBufferLevel level, uint32 count);
};

template<>
struct CmdBuffers<Dma> : public impl::CmdBufferCollectionBase {
	CmdBuffers() = default;
	explicit CmdBuffers(vk::CommandBufferLevel level, uint32 count);
};

template<typename T>
struct InFlightCmdBuffers : public CmdBuffers<T> {
	InFlightCmdBuffers() = default;
	explicit InFlightCmdBuffers(vk::CommandBufferLevel level)
		: CmdBuffers(level, c_framesInFlight)
	{
	}
};


} // namespace vl
