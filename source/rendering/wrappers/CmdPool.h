#pragma once

#include "Queue.h"

namespace vl {

struct CmdPool {

	const RQueue& queue;

	CmdPool(const RQueue& pQueue);
	[[nodiscard]] vk::CommandPool handle() const { return uHandle.get(); }

private:
	vk::UniqueCommandPool uHandle;
};
} // namespace vl
