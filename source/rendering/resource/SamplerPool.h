#pragma once

#include "rendering/VkCoreIncludes.h"

namespace vl {
class SamplerPool {

	struct SamplerParams {
		VkSamplerCreateInfo createInfo{};
		bool operator==(const SamplerParams& other) const { return memcmp(this, &other, sizeof(SamplerParams)) == 0; }
	};


	struct Entry {
		vk::Sampler sampler{};
		size_t nextFreeIndex{ UINT64_MAX };
		size_t refCount{ 0 };
		SamplerParams state;
	};

	size_t m_freeIndex{ UINT64_MAX };
	std::vector<Entry> m_entries;

	std::unordered_map<SamplerParams, size_t, Hash_fn<SamplerParams>> m_stateMap;
	std::unordered_map<vk::Sampler, size_t> m_samplerMap;

public:
	vk::Sampler AcquireSampler(const vk::SamplerCreateInfo& createInfo);

	// decrements ref-count and destroys sampler if possible
	void ReleaseSampler(vk::Sampler sampler);

	~SamplerPool();
};
} // namespace vl
