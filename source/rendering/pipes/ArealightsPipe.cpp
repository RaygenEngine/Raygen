#include "ArealightsPipe.h"

#include "engine/console/ConsoleVariable.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"
#include "rendering/util/WriteDescriptorSets.h"

namespace {
struct PushConstant {
	int32 frame;
	int32 pointlightCount;
	int32 quadlightCount;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
vk::UniquePipelineLayout ArealightsPipe::MakePipelineLayout()
{
	std::array layouts{
		Layouts->globalDescLayout.handle(),    // gbuffer and stuff
		Layouts->doubleStorageImage.handle(),  // image result
		Layouts->accelLayout.handle(),         // accel structure
		Layouts->singleStorageBuffer.handle(), // quadlights
	};

	// pipeline layout
	vk::PushConstantRange pushConstantRange{};
	pushConstantRange
		.setStageFlags(vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR) //
		.setSize(sizeof(PushConstant))
		.setOffset(0u);

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setPushConstantRanges(pushConstantRange) //
		.setSetLayouts(layouts);

	return Device->createPipelineLayoutUnique(pipelineLayoutInfo);
}

vk::UniquePipeline ArealightsPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader
		= GpuAssetManager->CompileShader("engine-data/spv/raytrace/arealights/arealights.shader");
	gpuShader.onCompileRayTracing = [&]() {
		StaticPipes::Recompile<ArealightsPipe>();
	};

	m_rtShaderGroups.clear();

	// Indices within this vector will be used as unique identifiers for the shaders in the Shader Binding Table.
	std::vector<vk::PipelineShaderStageCreateInfo> stages;

	// Raygen
	vk::RayTracingShaderGroupCreateInfoKHR rg{};
	rg.setType(vk::RayTracingShaderGroupTypeKHR::eGeneral) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	stages.push_back({ {}, vk::ShaderStageFlagBits::eRaygenKHR, *gpuShader.rayGen.Lock().module, "main" });
	rg.setGeneralShader(static_cast<uint32>(stages.size() - 1));

	m_rtShaderGroups.push_back(rg);

	// Miss
	vk::RayTracingShaderGroupCreateInfoKHR mg{};
	mg.setType(vk::RayTracingShaderGroupTypeKHR::eGeneral) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	stages.push_back({ {}, vk::ShaderStageFlagBits::eMissKHR, *gpuShader.miss.Lock().module, "main" });
	mg.setGeneralShader(static_cast<uint32>(stages.size() - 1));

	m_rtShaderGroups.push_back(mg);

	vk::RayTracingShaderGroupCreateInfoKHR hg{};
	hg.setType(vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	stages.push_back({ {}, vk::ShaderStageFlagBits::eClosestHitKHR, *gpuShader.closestHit.Lock().module, "main" });
	hg.setClosestHitShader(static_cast<uint32>(stages.size() - 1));

	m_rtShaderGroups.push_back(hg);


	// Assemble the shader stages and recursion depth info into the ray tracing pipeline
	vk::RayTracingPipelineCreateInfoKHR rayPipelineInfo{};
	rayPipelineInfo
		// Stages are shaders
		.setStages(stages);

	rayPipelineInfo
		// 1-raygen, n-miss, n-(hit[+anyhit+intersect])
		.setGroups(m_rtShaderGroups)
		// Note that it is preferable to keep the recursion level as low as possible, replacing it by a loop
		// formulation instead.

		.setMaxRecursionDepth(10) // Ray depth TODO:
		.setLayout(layout());

	auto pipeline = Device->createRayTracingPipelineKHRUnique({}, rayPipelineInfo);

	auto groupCount = static_cast<uint32>(m_rtShaderGroups.size());                  // 3 shaders: raygen, miss, chit
	uint32 groupHandleSize = Device->pd.raytracingProperties.shaderGroupHandleSize;  // Size of a program identifier
	uint32 baseAlignment = Device->pd.raytracingProperties.shaderGroupBaseAlignment; // Size of shader alignment

	// Fetch all the shader handles used in the pipeline, so that they can be written in the SBT
	uint32 sbtSize = groupCount * baseAlignment;

	std::vector<byte> shaderHandleStorage(sbtSize);
	Device->getRayTracingShaderGroupHandlesKHR(
		pipeline.value.get(), 0, groupCount, sbtSize, shaderHandleStorage.data());
	// Write the handles in the SBT
	m_rtSBTBuffer = RBuffer{ sbtSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	DEBUG_NAME(m_rtSBTBuffer.handle(), "Shader Binding Table");

	// TODO: Tidy
	auto mem = m_rtSBTBuffer.memory();

	void* dptr = Device->mapMemory(mem, 0, sbtSize);

	auto* pData = reinterpret_cast<uint8_t*>(dptr);
	for (uint32_t g = 0; g < groupCount; g++) {
		memcpy(pData, shaderHandleStorage.data() + g * groupHandleSize, groupHandleSize);
		pData += baseAlignment;
	}
	Device->unmapMemory(mem);

	return pipeline;
}

void ArealightsPipe::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc,
	vk::DescriptorSet storageImagesDescSet, const vk::Extent3D& extent, int32 frame) const
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 0u, 1u, &sceneDesc.globalDesc, 0u,
		nullptr); // gbuffer and stuff

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eRayTracingKHR, layout(), 1u, 1u, &storageImagesDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eRayTracingKHR, layout(), 2u, 1u, &sceneDesc.scene->sceneAsDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 3u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSetQuadlights[sceneDesc.frameIndex], 0u, nullptr);

	static int32 frameIndex = 0;

	PushConstant pc{
		frame,
		sceneDesc.scene->tlas.sceneDesc.quadlightCount,
		sceneDesc.scene->tlas.sceneDesc.quadlightCount,
	};

	cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR, 0u,
		sizeof(PushConstant), &pc);

	vk::DeviceSize progSize = Device->pd.raytracingProperties.shaderGroupBaseAlignment; // Size of a program identifier

	// RayGen index
	vk::DeviceSize rayGenOffset = 0u * progSize; // Start at the beginning of m_sbtBuffer

	// Miss index
	vk::DeviceSize missOffset = 1u * progSize; // Jump over raygen
	vk::DeviceSize missStride = progSize;

	// Hit index
	vk::DeviceSize hitGroupOffset = 2u * progSize; // Jump over the previous shaders
	vk::DeviceSize hitGroupStride = progSize;


	// We can finally call traceRaysKHR that will add the ray tracing launch in the command buffer. Note that the
	// SBT buffer is mentioned several times. This is due to the possibility of separating the SBT into several
	// buffers, one for each type: ray generation, miss shaders, hit groups, and callable shaders (outside the scope
	// of this tutorial). The last three parameters are equivalent to the grid size of a compute launch, and
	// represent the total number of threads. Since we want to trace one ray per pixel, the grid size has the width
	// and height of the output image, and a depth of 1.

	vk::DeviceSize sbtSize = progSize * (vk::DeviceSize)m_rtShaderGroups.size();

	const vk::StridedBufferRegionKHR raygenShaderBindingTable
		= { m_rtSBTBuffer.handle(), rayGenOffset, progSize, sbtSize };
	const vk::StridedBufferRegionKHR missShaderBindingTable = { m_rtSBTBuffer.handle(), missOffset, progSize, sbtSize };
	const vk::StridedBufferRegionKHR hitShaderBindingTable
		= { m_rtSBTBuffer.handle(), hitGroupOffset, progSize, sbtSize };
	const vk::StridedBufferRegionKHR callableShaderBindingTable;

	cmdBuffer.traceRaysKHR(&raygenShaderBindingTable, &missShaderBindingTable, &hitShaderBindingTable,
		&callableShaderBindingTable, extent.width, extent.height, 1);
}

} // namespace vl