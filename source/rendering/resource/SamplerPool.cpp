#include "SamplerPool.h"

#include "rendering/Device.h"

namespace vl {
vk::Sampler SamplerPool::AcquireSampler(const vk::SamplerCreateInfo& createInfo)
{
	SamplerParams state;
	state.createInfo = createInfo;

	auto it = m_stateMap.find(state);
	if (it == m_stateMap.end()) {
		size_t index{};
		if (m_freeIndex != UINT64_MAX) {
			index = m_freeIndex;
			m_freeIndex = m_entries[index].nextFreeIndex;
		}
		else {
			index = m_entries.size();
			m_entries.resize(m_entries.size() + 1);
		}

		vk::Sampler sampler = Device->createSampler(createInfo);

		m_entries[index].refCount = 1;
		m_entries[index].sampler = sampler;
		m_entries[index].state = state;

		m_stateMap.insert({ state, index });
		m_samplerMap.insert({ sampler, index });

		return sampler;
	}
	else {
		++m_entries[it->second].refCount;
		return m_entries[it->second].sampler;
	}
}

void SamplerPool::ReleaseSampler(vk::Sampler sampler)
{
	auto it = m_samplerMap.find(sampler);
	CLOG_ABORT(it == m_samplerMap.end(), "Could not find sampler to release in sampler pool");

	size_t index = it->second;
	Entry& entry = m_entries[index];

	CLOG_ABORT(entry.sampler != sampler, "Sampler entry / handle missmatch in sampler pool");
	CLOG_ABORT(entry.refCount == 0, "Attempting to release a sampler entry with zero refs");

	entry.refCount--;

	if (entry.refCount == 0) {
		Device->destroySampler(sampler);

		entry.sampler = nullptr;
		entry.nextFreeIndex = m_freeIndex;
		m_freeIndex = index;

		m_stateMap.erase(entry.state);
		m_samplerMap.erase(sampler);
	}
}

SamplerPool::~SamplerPool()
{
	for (auto it : m_entries) {
		if (it.sampler) {
			Device->destroySampler(it.sampler);
		}
	}
}
} // namespace vl
