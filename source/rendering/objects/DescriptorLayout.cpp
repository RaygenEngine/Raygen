#include "pch.h"
#include "DescriptorLayout.h"

#include "engine/Logger.h"
#include "rendering/Device.h"
#include "rendering/resource/GpuResources.h"

namespace detail {
template<class T>
inline void hash_combine(size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct PoolHasher {
	size_t operator()(const vl::DescriptorLayout& layoutSize)
	{
		size_t hash = 0;
		for (auto& size : layoutSize.perSetPoolSizes) {
			hash_combine(hash, size.descriptorCount);
			hash_combine(hash, size.type);
		}
		return hash;
	}
};
} // namespace detail

namespace vl {
void DescriptorLayout::AddBinding(vk::DescriptorType type, vk::ShaderStageFlags stageFlags, uint32 descriptorCount)
{
	CLOG_ABORT(hasBeenGenerated, "Attempting to add binding to an R_DescriptorLayout that is already generated");

	vk::DescriptorSetLayoutBinding binding{};
	binding
		.setBinding(static_cast<uint32>(bindings.size())) //
		.setDescriptorType(type)
		.setDescriptorCount(descriptorCount)
		.setStageFlags(stageFlags)
		.setPImmutableSamplers(nullptr);

	bindings.push_back(binding);

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

void DescriptorLayout::Generate()
{
	CLOG_ABORT(hasBeenGenerated, "Attempting to generate an R_DescriptorLayout that is already generated");

	vk::DescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo
		.setBindingCount(static_cast<uint32>(bindings.size())) //
		.setPBindings(bindings.data());

	hasBeenGenerated = true;
	setLayout = Device->createDescriptorSetLayoutUnique(layoutInfo);
	poolSizeHash = detail::PoolHasher{}(*this);
}

vk::DescriptorSet DescriptorLayout::GetDescriptorSet() const
{
	CLOG_ABORT(!hasBeenGenerated, "Attempting to get a descriptor set from a non generated R_DescriptorLayout ");
	return GpuResources->descPools.AllocateDescriptorSet(poolSizeHash, *this);
}
} // namespace vl
