#include "pch.h"
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
void RDescriptorSetLayout::AddBinding(vk::DescriptorType type, vk::ShaderStageFlags stageFlags, uint32 descriptorCount)
{
	CLOG_ABORT(hasBeenGenerated, "Attempting to add binding to an DescriptorLayout that is already generated");

	vk::DescriptorSetLayoutBinding binding{};
	binding
		.setBinding(static_cast<uint32>(bindings.size())) //
		.setDescriptorType(type)
		.setDescriptorCount(descriptorCount)
		.setStageFlags(stageFlags)
		.setPImmutableSamplers(nullptr);

	bindings.push_back(binding);

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

void RDescriptorSetLayout::Generate()
{
	CLOG_ABORT(hasBeenGenerated, "Attempting to generate a DescriptorLayout that is already generated");

	vk::DescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.setBindings(bindings);

	hasBeenGenerated = true;
	uHandle = Device->createDescriptorSetLayoutUnique(layoutInfo);
	poolSizeHash = PoolHasher{}(perSetPoolSizes);
}

vk::DescriptorSet RDescriptorSetLayout::AllocDescriptorSet() const
{
	CLOG_ABORT(!hasBeenGenerated, "Attempting to get a descriptor set from a non generated DescriptorLayout");
	return GpuResources::AllocateDescriptorSet(poolSizeHash, *this);
}

} // namespace vl