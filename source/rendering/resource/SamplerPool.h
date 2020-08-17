#pragma once

namespace vl {
class SamplerPool {

    struct SamplerState
    {
        VkSamplerCreateInfo                createInfo{};

        SamplerState() { memset(this, 0, sizeof(SamplerState)); }

        bool operator==(const SamplerState& other) const { return memcmp(this, &other, sizeof(SamplerState)) == 0; }
    };


    struct Entry
    {
        vk::Sampler    sampler{};
        uint32     nextFreeIndex = ~0;
        uint32     refCount = 0;
        SamplerState state;
    };

    uint32           m_freeIndex = ~0;
    std::vector<Entry> m_entries;

    std::unordered_map<SamplerState, uint32, Hash_fn<SamplerState>> m_stateMap;
    std::unordered_map<vk::Sampler, uint32>             m_samplerMap;

public:

    vk::Sampler AcquireSampler(const vk::SamplerCreateInfo& createInfo);

    // decrements ref-count and destroys sampler if possible
    void ReleaseSampler(vk::Sampler sampler);

    ~SamplerPool();
};
} // namespace vl
