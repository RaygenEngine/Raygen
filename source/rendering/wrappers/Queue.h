#pragma once

namespace vl {
struct RQueue : public vk::Queue {
	struct Family {
		vk::QueueFamilyProperties props;
		uint32 index{ UINT_MAX };
		bool supportsPresent{ false };
	};

	const Family& family;

	RQueue(const Family& fam);
};

} // namespace vl
