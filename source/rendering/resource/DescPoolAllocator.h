#pragma once
#include "rendering/wrappers/DescriptorSetLayout.h"

namespace vl {
class DescPoolAllocator {
	static constexpr size_t c_setsPerPool = 100;
	struct Entry {
		std::vector<vk::DescriptorPoolSize> poolSizes;
		std::vector<vk::UniqueDescriptorPool> pools;
		size_t allocated{ 0 };
	};

	std::unordered_map<size_t, Entry> m_entries;

	vk::UniqueDescriptorPool m_imguiPool;

	size_t m_allocCount{ 0 };

public:
	vk::DescriptorSet AllocateDescriptorSet(size_t hash, const RDescriptorSetLayout& layout, int32 bindingSize = -1);

	vk::DescriptorPool GetImguiPool();

	size_t GetAllocations() { return m_allocCount; }
};
} // namespace vl
