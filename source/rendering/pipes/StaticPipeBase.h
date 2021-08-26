#pragma once
#include "rendering/wrappers/Buffer.h"

#include "rendering/VkCoreIncludes.h"

// Fwd declared here for subclasses
struct SceneRenderDesc;

namespace vl {
// Follows RAII rules, use custom constructor / destructor if you need more init/cleanup
class StaticPipeBase {
private:
	friend class StaticPipes;
	vk::UniquePipelineLayout m_layout;
	vk::UniquePipeline m_pipeline;

public:
	StaticPipeBase(StaticPipeBase const&) = delete;
	StaticPipeBase(StaticPipeBase&&) = delete;
	StaticPipeBase& operator=(StaticPipeBase const&) = delete;
	StaticPipeBase& operator=(StaticPipeBase&&) = delete;


protected:
	virtual vk::UniquePipelineLayout MakePipelineLayout() = 0;
	virtual vk::UniquePipeline MakePipeline() = 0;

public:
	StaticPipeBase() = default;
	virtual ~StaticPipeBase() = default;

	vk::Pipeline pipeline() const { return *m_pipeline; }
	vk::PipelineLayout layout() const { return *m_layout; }
};


class StaticRaytracingPipeBase : public StaticPipeBase {

	RBuffer m_rtSBTBuffer;
	std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_rtShaderGroups;
	std::vector<vk::PipelineShaderStageCreateInfo> m_stages;
	size_t missCount{};


protected:
	vk::StridedDeviceAddressRegionKHR m_raygenShaderBindingTable;
	vk::StridedDeviceAddressRegionKHR m_missShaderBindingTable;
	vk::StridedDeviceAddressRegionKHR m_hitShaderBindingTable;
	vk::StridedDeviceAddressRegionKHR m_callableShaderBindingTable;

	void AddRaygenGroup(vk::ShaderModule shader);
	void AddMissGroup(vk::ShaderModule shader);
	void AddHitGroup(vk::ShaderModule chit, vk::ShaderModule ahit = {});
	vk::UniquePipeline MakeRtPipeline(vk::RayTracingPipelineCreateInfoKHR& createInfo);
};

} // namespace vl
