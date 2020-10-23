#include "Queue.h"

#include "rendering/Device.h"

namespace vl {
RQueue::RQueue(const Family& fam)
	: family(fam)
{
	auto queue = Device->getQueue(family.index, 0);
	vk::Queue::operator=(queue);
}
} // namespace vl
