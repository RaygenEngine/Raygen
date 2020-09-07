#include "pch.h"
#include "GpuResources.h"

#include "rendering/resource/DescPoolAllocator.h"
#include "rendering/resource/SamplerPool.h"

namespace {
vl::DescPoolAllocator* descPools;
vl::SamplerPool* samplerPool;
} // namespace

namespace vl {

void GpuResources::Init()
{
	descPools = new DescPoolAllocator();
	samplerPool = new SamplerPool();
}

vk::DescriptorSet GpuResources::AllocateDescriptorSet(
	size_t hash, const RDescriptorSetLayout& layout, int32 variableBindingSize)
{
	return descPools->AllocateDescriptorSet(hash, layout, variableBindingSize);
}

vk::DescriptorPool GpuResources::GetImguiPool()
{
	return descPools->GetImguiPool();
}

vk::Sampler GpuResources::AcquireSampler(const vk::SamplerCreateInfo& createInfo)
{
	return samplerPool->AcquireSampler(createInfo);
}

void GpuResources::ReleaseSampler(vk::Sampler sampler)
{
	samplerPool->ReleaseSampler(sampler);
}

size_t GpuResources::GetAllocations()
{
	return descPools->GetAllocations();
}

void GpuResources::Destroy()
{
	delete descPools;
	descPools = nullptr;
	delete samplerPool;
	samplerPool = nullptr;
}
} // namespace vl
