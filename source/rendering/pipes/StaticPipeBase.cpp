#include "StaticPipeBase.h"

#include "rendering/Device.h"

namespace vl {
void StaticRaytracingPipeBase::AddRaygenGroup(vk::ShaderModule shader)
{
	m_rtShaderGroups.clear();
	m_stages.clear();

	vk::RayTracingShaderGroupCreateInfoKHR rg{};
	rg.setType(vk::RayTracingShaderGroupTypeKHR::eGeneral) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	m_stages.push_back({ {}, vk::ShaderStageFlagBits::eRaygenKHR, shader, "main" });
	rg.setGeneralShader(static_cast<uint32>(m_stages.size() - 1));

	m_rtShaderGroups.push_back(rg);
}

void StaticRaytracingPipeBase::AddMissGroup(vk::ShaderModule shader)
{
	vk::RayTracingShaderGroupCreateInfoKHR mg{};
	mg.setType(vk::RayTracingShaderGroupTypeKHR::eGeneral) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	m_stages.push_back({ {}, vk::ShaderStageFlagBits::eMissKHR, shader, "main" });
	mg.setGeneralShader(static_cast<uint32>(m_stages.size() - 1));

	m_rtShaderGroups.push_back(mg);
	missCount++;
}

void StaticRaytracingPipeBase::AddHitGroup(vk::ShaderModule chit, vk::ShaderModule ahit)
{
	vk::RayTracingShaderGroupCreateInfoKHR hg{};
	hg.setType(vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	m_stages.push_back({ {}, vk::ShaderStageFlagBits::eClosestHitKHR, chit, "main" });
	hg.setClosestHitShader(static_cast<uint32>(m_stages.size() - 1));

	if (ahit) {
		m_stages.push_back({ {}, vk::ShaderStageFlagBits::eAnyHitKHR, ahit, "main" });
		hg.setAnyHitShader(static_cast<uint32>(m_stages.size() - 1));
	}

	m_rtShaderGroups.push_back(hg);
}

vk::UniquePipeline StaticRaytracingPipeBase::MakeRtPipeline(vk::RayTracingPipelineCreateInfoKHR& createInfo)
{
	// Stages are shaders
	createInfo
		.setStages(m_stages)
		// 1-raygen, n-miss, n-(hit[+anyhit+intersect])
		.setGroups(m_rtShaderGroups);


	auto pipeline = Device->createRayTracingPipelineKHRUnique({}, {}, createInfo);

	auto groupCount = static_cast<uint32>(m_rtShaderGroups.size());
	uint32 groupHandleSize = Device->pd.raytracingProperties.shaderGroupHandleSize;  // Size of a program identifier
	uint32 baseAlignment = Device->pd.raytracingProperties.shaderGroupBaseAlignment; // Size of shader alignment

	// Fetch all the shader handles used in the pipeline, so that they can be written in the SBT
	uint32 sbtSize = groupCount * baseAlignment;

	std::vector<byte> shaderHandleStorage(sbtSize);
	(void)Device->getRayTracingShaderGroupHandlesKHR(
		pipeline.value.get(), 0, groupCount, sbtSize, shaderHandleStorage.data());
	// Write the handles in the SBT
	m_rtSBTBuffer = RBuffer{ sbtSize,
		vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderBindingTableKHR
			| vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		vk::MemoryAllocateFlagBits::eDeviceAddress };

	DEBUG_NAME(m_rtSBTBuffer.handle(), "Shader Binding Table");


	// at the beginning of SBT
	m_raygenShaderBindingTable
		= vk::StridedDeviceAddressRegionKHR{ m_rtSBTBuffer.address(), baseAlignment, baseAlignment };

	// after the only raygen
	m_missShaderBindingTable
		= vk::StridedDeviceAddressRegionKHR{ m_rtSBTBuffer.address() + baseAlignment, baseAlignment, baseAlignment };

	// after raygen + miss shaders
	vk::DeviceSize hitGroupOffset = baseAlignment + missCount * baseAlignment;
	m_hitShaderBindingTable
		= vk::StridedDeviceAddressRegionKHR{ m_rtSBTBuffer.address() + hitGroupOffset, baseAlignment, baseAlignment };


	auto mem = m_rtSBTBuffer.memory();

	void* dptr = Device->mapMemory(mem, 0, sbtSize);

	auto* pData = reinterpret_cast<uint8_t*>(dptr);
	for (uint32_t g = 0; g < groupCount; g++) {
		memcpy(pData, shaderHandleStorage.data() + g * groupHandleSize, groupHandleSize);
		pData += baseAlignment;
	}
	Device->unmapMemory(mem);

	return std::move(pipeline.value);
}
} // namespace vl
