#include "DescriptorSetLayout.h"

#include "engine/Logger.h"
#include "rendering/Device.h"
#include "rendering/resource/GpuResources.h"

namespace {
template<class T>
inline void hash_combine(size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct PoolHasher {
	size_t operator()(const std::vector<vk::DescriptorPoolSize>& perSetPoolSizes)
	{
		size_t hash = 0;
		for (auto& size : perSetPoolSizes) {
			hash_combine(hash, size.descriptorCount);
			hash_combine(hash, size.type);
		}
		return hash;
	}
};
} // namespace

namespace vl {
void RDescriptorSetLayout::AddBinding(vk::DescriptorType type, vk::ShaderStageFlags stageFlags, uint32 descriptorCount,
	vk::DescriptorBindingFlags inBindingFlags)
{
	CLOG_ABORT(hasBeenGenerated, "Attempting to add binding to an DescriptorLayout that is already generated");

	// Should be abort, but we prefer vulkan validation errors instead of aborting here.
	CLOG_ERROR(hasVariableDescriptorCountBinding,
		"Attempting to add bindings after variable descriptor count binding. "
		"This will produce an invalid descriptor set layout and should be an abort but the program will continue to "
		"let validation spit better errors.");

	const bool hasVariableDescCount
		= (inBindingFlags & vk::DescriptorBindingFlagBits::eVariableDescriptorCount).operator bool();

	hasVariableDescriptorCountBinding |= hasVariableDescCount;

	vk::DescriptorSetLayoutBinding binding{};
	binding
		.setBinding(static_cast<uint32>(bindings.size())) //
		.setDescriptorType(type)
		.setDescriptorCount(descriptorCount)
		.setStageFlags(stageFlags)
		.setPImmutableSamplers(nullptr);

	bindings.emplace_back(binding);
	bindingFlags.emplace_back(inBindingFlags);


	if (type == vk::DescriptorType::eUniformBuffer || type == vk::DescriptorType::eUniformBufferDynamic) {
		hasUbo = true;
	}

	auto it
		= std::find_if(perSetPoolSizes.begin(), perSetPoolSizes.end(), [&](auto& size) { return size.type == type; });

	if (it != perSetPoolSizes.end()) {
		it->descriptorCount++;
	}
	else {
		vk::DescriptorPoolSize size{};
		size.setDescriptorCount(1) //
			.setType(type);

		perSetPoolSizes.push_back(size);
	}
}

void RDescriptorSetLayout::AddBinding(
	vk::DescriptorType type, vk::ShaderStageFlags stageFlags, vk::DescriptorBindingFlags inBindingFlags)
{
	AddBinding(type, stageFlags, 1u, inBindingFlags);
}

void RDescriptorSetLayout::Generate()
{
	CLOG_ABORT(hasBeenGenerated, "Attempting to generate a DescriptorLayout that is already generated");

	vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCI;
	bindingFlagsCI.setBindingFlags(bindingFlags);

	vk::DescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo //
		.setBindings(bindings)
		.setPNext(&bindingFlagsCI);


	hasBeenGenerated = true;
	uHandle = Device->createDescriptorSetLayoutUnique(layoutInfo);
	poolSizeHash = PoolHasher{}(perSetPoolSizes);
}

vk::DescriptorSet RDescriptorSetLayout::AllocDescriptorSet(int32 variableBindingSize) const
{
	CLOG_ABORT(variableBindingSize >= 0 && !hasVariableDescriptorCountBinding,
		"Passed size parameter to allocation of descriptor set that does not have variable descriptor binding.");

	CLOG_ABORT(variableBindingSize < 0 && hasVariableDescriptorCountBinding,
		"Did not give a variable binding size for descriptor that has a variable count binding.");
	CLOG_ABORT(!hasBeenGenerated, "Attempting to get a descriptor set from a non generated DescriptorLayout");
	return GpuResources::AllocateDescriptorSet(poolSizeHash, *this, variableBindingSize);
}

vk::UniqueDescriptorSet RDescriptorSetLayout::AllocDescriptorSetUnique(int32 variableBindingSize) const
{
	CLOG_ABORT(variableBindingSize >= 0 && !hasVariableDescriptorCountBinding,
		"Passed size parameter to allocation of descriptor set that does not have variable descriptor binding.");

	CLOG_ABORT(variableBindingSize < 0 && hasVariableDescriptorCountBinding,
		"Did not give a variable binding size for descriptor that has a variable count binding.");
	CLOG_ABORT(!hasBeenGenerated, "Attempting to get a descriptor set from a non generated DescriptorLayout");
	return GpuResources::AllocateDescriptorSetUnique(poolSizeHash, *this, variableBindingSize);
}

} // namespace vl
