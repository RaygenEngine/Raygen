#include "pch.h"
#include "renderer/PoolAllocator.h"

#include "engine/console/ConsoleVariable.h"
#include "engine/Logger.h"
#include "renderer/VulkanLayer.h"
#include "renderer/wrapper/Device.h"

ConsoleFunction<> g_showPoolAllocations{ "r.mem.showDescriptorPools",
	[]() { LOG_REPORT("Pools: {}", Layer->poolAllocator.GetAllocations()); },
	"Shows number of allocated descriptor pools." };


namespace detail {
template<class T>
inline void hash_combine(size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct PoolHasher {
	size_t operator()(const R_DescriptorLayout& layoutSize)
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

void R_DescriptorLayout::AddBinding(vk::DescriptorType type, vk::ShaderStageFlags stageFlags, uint32 descriptorCount)
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

void R_DescriptorLayout::Generate()
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

vk::DescriptorSet R_DescriptorLayout::GetDescriptorSet() const
{
	CLOG_ABORT(!hasBeenGenerated, "Attempting to get a descriptor set from a non generated R_DescriptorLayout ");
	return Layer->poolAllocator.AllocateDescritporSet(poolSizeHash, *this);
}


vk::DescriptorSet PoolAllocator::AllocateDescritporSet(size_t hash, const R_DescriptorLayout& layout)
{
	auto addPool = [&](Entry& entry) {
		vk::DescriptorPoolCreateInfo poolInfo{};
		poolInfo
			.setPoolSizeCount(static_cast<uint32>(entry.poolSizes.size())) //
			.setPPoolSizes(entry.poolSizes.data())
			.setMaxSets(c_setsPerPool);

		entry.pools.emplace_back(std::move(Device->createDescriptorPoolUnique(poolInfo)));
		entry.allocated = 0;
		allocCount++;
	};

	auto it = entries.find(hash);

	if (it == entries.end()) {
		Entry e{};
		e.poolSizes = layout.perSetPoolSizes;
		for (auto& poolSize : e.poolSizes) {
			poolSize.descriptorCount *= c_setsPerPool;
		}
		addPool(e);
		it = entries.emplace(hash, std::move(e)).first;
	}

	auto& entry = it->second;

	entry.allocated++;
	if (entry.allocated >= c_setsPerPool) {
		addPool(entry);
	}


	vk::DescriptorSetAllocateInfo allocInfo{};
	allocInfo //
		.setDescriptorPool(entry.pools.back().get())
		.setDescriptorSetCount(1)
		.setPSetLayouts(&layout.setLayout.get());

	return Device->allocateDescriptorSets(allocInfo)[0];
}

vk::DescriptorPool PoolAllocator::GetImguiPool()
{
	if (!imguiPool) {
		vk::DescriptorSetLayoutBinding binding{};
		binding
			.setBinding(1u) //
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1u)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment)
			.setPImmutableSamplers(nullptr);

		vk::DescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo
			.setBindingCount(1u) //
			.setPBindings(&binding);

		vk::DescriptorPoolSize poolSize;
		poolSize
			.setDescriptorCount(2u) //
			.setType(vk::DescriptorType::eCombinedImageSampler);

		vk::DescriptorPoolCreateInfo poolInfo{};
		poolInfo
			.setPoolSizeCount(1u) //
			.setPPoolSizes(&poolSize)
			.setMaxSets(2u);

		imguiPool = std::move(Device->createDescriptorPoolUnique(poolInfo));
	}
	return *imguiPool;
}
