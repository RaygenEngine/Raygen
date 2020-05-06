#pragma once
#include <vulkan/vulkan.hpp>

namespace vl {
struct RDescriptorLayout {
	
	std::vector<vk::DescriptorSetLayoutBinding> bindings;
	std::vector<vk::DescriptorPoolSize> perSetPoolSizes;

	vk::UniqueDescriptorSetLayout setLayout;

	bool hasBeenGenerated{ false };

	void AddBinding(vk::DescriptorType type, vk::ShaderStageFlags stageFlags, uint32 descriptorCount = 1u);

	void Generate();

	[[nodiscard]] vk::DescriptorSet GetDescriptorSet() const;

private:
	size_t poolSizeHash;
};
} // namespace vl
