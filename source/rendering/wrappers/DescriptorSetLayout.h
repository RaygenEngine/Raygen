#pragma once

namespace vl {
struct RDescriptorSetLayout {


	void AddBinding(vk::DescriptorType type, vk::ShaderStageFlags stageFlags, uint32 descriptorCount = 1u,
		vk::DescriptorBindingFlags inBindingFlags = {});

	void AddBinding(
		vk::DescriptorType type, vk::ShaderStageFlags stageFlags, vk::DescriptorBindingFlags inBindingFlags);


	void Generate();

	[[nodiscard]] vk::DescriptorSetLayout handle() const { return uHandle.get(); };

	[[nodiscard]] vk::DescriptorSet AllocDescriptorSet(int32 dynamicBindingSize = -1) const;
	[[nodiscard]] vk::UniqueDescriptorSet AllocDescriptorSetUnique(int32 dynamicBindingSize = -1) const;
	[[nodiscard]] bool IsEmpty() const { return bindings.empty(); }
	[[nodiscard]] bool HasUbo() const { return hasUbo; }
	[[nodiscard]] bool HasBeenGenerated() const { return hasBeenGenerated; }
	[[nodiscard]] const std::vector<vk::DescriptorSetLayoutBinding>& GetBindings() const { return bindings; }
	[[nodiscard]] const std::vector<vk::DescriptorPoolSize>& GetPerSetPoolSizes() const { return perSetPoolSizes; }

private:
	vk::UniqueDescriptorSetLayout uHandle;

	std::vector<vk::DescriptorSetLayoutBinding> bindings;
	std::vector<vk::DescriptorPoolSize> perSetPoolSizes;

	std::vector<vk::DescriptorBindingFlags> bindingFlags;

	size_t poolSizeHash;

	bool hasUbo{ false };
	bool hasBeenGenerated{ false };
	bool hasVariableDescriptorCountBinding{ false };
};
} // namespace vl
