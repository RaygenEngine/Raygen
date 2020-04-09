#pragma once
#include "rendering/objects/DescriptorLayout.h"

#include <vulkan/vulkan.hpp>
#include <unordered_map>

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
	vk::DescriptorSet AllocateDescriptorSet(size_t hash, const DescriptorLayout& layout);

	vk::DescriptorPool GetImguiPool();

	size_t GetAllocations() { return m_allocCount; }
};
} // namespace vl
