#include "pch.h"
#include "DescPoolAllocator.h"

#include "engine/console/ConsoleVariable.h"
#include "rendering/Device.h"
#include "rendering/resource/GpuResources.h"

ConsoleFunction<> g_showPoolAllocations{ "r.mem.showDescriptorPools",
	[]() { LOG_REPORT("Pools: {}", vl::GpuResources::GetAllocations()); },
	"Shows number of allocated descriptor pools." };

namespace vl {
vk::DescriptorSet DescPoolAllocator::AllocateDescriptorSet(size_t hash, const RDescriptorLayout& layout)
{
	auto addPool = [&](Entry& entry) {
		vk::DescriptorPoolCreateInfo poolInfo{};
		poolInfo
			.setPoolSizeCount(static_cast<uint32>(entry.poolSizes.size())) //
			.setPPoolSizes(entry.poolSizes.data())
			.setMaxSets(c_setsPerPool);

		entry.pools.emplace_back(std::move(Device->createDescriptorPoolUnique(poolInfo)));
		entry.allocated = 0;
		m_allocCount++;
	};

	auto it = m_entries.find(hash);

	if (it == m_entries.end()) {
		Entry e{};
		e.poolSizes = layout.perSetPoolSizes;
		for (auto& poolSize : e.poolSizes) {
			poolSize.descriptorCount *= c_setsPerPool;
		}
		addPool(e);
		it = m_entries.emplace(hash, std::move(e)).first;
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

vk::DescriptorPool DescPoolAllocator::GetImguiPool()
{
	if (!m_imguiPool) {
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

		m_imguiPool = std::move(Device->createDescriptorPoolUnique(poolInfo));
	}
	return *m_imguiPool;
}
} // namespace vl
