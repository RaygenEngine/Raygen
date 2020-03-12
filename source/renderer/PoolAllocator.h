#pragma once


#include <vulkan/vulkan.hpp>
#include <unordered_map>


struct R_DescriptorLayout {
private:
	size_t poolSizeHash;

public:
	std::vector<vk::DescriptorSetLayoutBinding> bindings;
	std::vector<vk::DescriptorPoolSize> perSetPoolSizes;

	vk::UniqueDescriptorSetLayout setLayout;

	bool hasBeenGenerated{ false };


	void AddBinding(vk::DescriptorType type, vk::ShaderStageFlags stageFlags, uint32 descriptorCount = 1u);

	void Generate();

	vk::DescriptorSet GetDescriptorSet() const;
};


class PoolAllocator {
	static constexpr size_t c_setsPerPool = 100;
	struct Entry {
		std::vector<vk::DescriptorPoolSize> poolSizes;
		std::vector<vk::UniqueDescriptorPool> pools;
		size_t allocated{ 0 };
	};

	std::unordered_map<size_t, Entry> entries;

	vk::UniqueDescriptorPool imguiPool;

	size_t allocCount{ 0 };

public:
	vk::DescriptorSet AllocateDescritporSet(size_t hash, const R_DescriptorLayout& layout);

	vk::DescriptorPool GetImguiPool();

	size_t GetAllocations() { return allocCount; }
};
