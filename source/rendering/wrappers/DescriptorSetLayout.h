#pragma once

namespace vl {
struct RDescriptorSetLayout {

	void AddBinding(vk::DescriptorType type, vk::ShaderStageFlags stageFlags, uint32 descriptorCount = 1u);
	void Generate();

	[[nodiscard]] vk::DescriptorSetLayout handle() const { return uHandle.get(); };

	[[nodiscard]] vk::DescriptorSet AllocDescriptorSet() const;
	[[nodiscard]] bool IsEmpty() const { return bindings.empty(); }
	[[nodiscard]] bool HasUbo() const { return hasUbo; }
	[[nodiscard]] bool HasBeenGenerated() const { return hasBeenGenerated; }
	[[nodiscard]] const std::vector<vk::DescriptorSetLayoutBinding>& GetBindings() const { return bindings; }
	[[nodiscard]] const std::vector<vk::DescriptorPoolSize>& GetPerSetPoolSizes() const { return perSetPoolSizes; }

private:
	vk::UniqueDescriptorSetLayout uHandle;

	std::vector<vk::DescriptorSetLayoutBinding> bindings;
	std::vector<vk::DescriptorPoolSize> perSetPoolSizes;

	size_t poolSizeHash;

	bool hasUbo{ false };
	bool hasBeenGenerated{ false };
};
} // namespace vl
