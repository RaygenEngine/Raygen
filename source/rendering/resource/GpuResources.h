#pragma once
#include "rendering/wrappers/DescriptorLayout.h"

namespace vl {
struct GpuResources {

	static vk::DescriptorSet AllocateDescriptorSet(size_t hash, const RDescriptorLayout& layout);
	static vk::DescriptorPool GetImguiPool();

	static vk::Sampler AcquireSampler(const vk::SamplerCreateInfo & createInfo);

	// decrements ref-count and destroys sampler if possible
	static void ReleaseSampler(vk::Sampler sampler);

	static size_t GetAllocations();

	static void Init();
	static void Destroy();
};
} // namespace vl
