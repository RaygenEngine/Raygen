#include "CmdPool.h"

#include "rendering/Device.h"

namespace vl {
CmdPool::CmdPool(const RQueue& pQueue)
	: queue(pQueue)
{
	vk::CommandPoolCreateInfo poolInfo{};
	poolInfo.setQueueFamilyIndex(queue.family.index);
	poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	uHandle = Device->createCommandPoolUnique(poolInfo);
}
} // namespace vl
