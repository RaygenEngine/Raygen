#include "PathtracedCubemapArray.h"

#include "rendering/Renderer.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/scene/SceneIrradianceGrid.h"
#include "rendering/scene/SceneReflProbe.h"
#include "rendering/wrappers/ImageView.h"

namespace {
struct PushConstant {
	int32 samples;
	int32 bounces;
	int32 pointlightCount;
	int32 spotlightCount;
	int32 dirlightCount;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
PathtracedCubemapArray::PathtracedCubemapArray()
{
	MakeRtPipeline();
}

void PathtracedCubemapArray::RecordPass(
	vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const SceneIrradianceGrid& ig)
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, m_pipeline.get());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_pipelineLayout.get(), 2u, 1u,
		&sceneDesc.scene->sceneAsDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_pipelineLayout.get(), 3u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSet[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_pipelineLayout.get(), 4u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSetPointlights[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_pipelineLayout.get(), 5u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSetSpotlights[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_pipelineLayout.get(), 6u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSetDirlights[sceneDesc.frameIndex], 0u, nullptr);

	vk::DeviceSize progSize = Device->pd.rtProps.shaderGroupBaseAlignment; // Size of a program identifier

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

	const vk::StridedBufferRegionKHR raygenShaderBindingTable{ m_rtSBTBuffer.handle(), rayGenOffset, progSize,
		sbtSize };
	const vk::StridedBufferRegionKHR missShaderBindingTable{ m_rtSBTBuffer.handle(), missOffset, progSize, sbtSize };
	const vk::StridedBufferRegionKHR hitShaderBindingTable{ m_rtSBTBuffer.handle(), hitGroupOffset, progSize, sbtSize };
	const vk::StridedBufferRegionKHR callableShaderBindingTable;

	for (auto ig : sceneDesc->Get<SceneIrradianceGrid>()) {

		ig->environmentCubemaps.TransitionToLayout(cmdBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
			vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eRayTracingShaderKHR);


		PushConstant pc{
			ig->ptSamples,
			ig->ptBounces,
			sceneDesc.scene->tlas.sceneDesc.pointlightCount,
			sceneDesc.scene->tlas.sceneDesc.spotlightCount,
			sceneDesc.scene->tlas.sceneDesc.dirlightCount,
		};

		cmdBuffer.pushConstants(m_pipelineLayout.get(),
			vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR, 0u, sizeof(PushConstant),
			&pc);

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_pipelineLayout.get(), 0u, 1u,
			&ig->environmentStorageDescSet, 0u, nullptr);

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_pipelineLayout.get(), 1u, 1u,
			&ig->uboDescSet[sceneDesc.frameIndex], 0u, nullptr);

		cmdBuffer.traceRaysKHR(&raygenShaderBindingTable, &missShaderBindingTable, &hitShaderBindingTable,
			&callableShaderBindingTable, ig->resolution, ig->resolution, 1);
	}
}

void PathtracedCubemapArray::MakeRtPipeline()
{
	std::array layouts{
		Layouts->cubemapArrayStorage.handle(),
		Layouts->singleUboDescLayout.handle(),
		Layouts->accelLayout.handle(),
		Layouts->bufferAndSamplersDescLayout.handle(),
		Layouts->singleStorageBuffer.handle(),
		Layouts->bufferAndSamplersDescLayout.handle(),
		Layouts->bufferAndSamplersDescLayout.handle(),
	};

	// all rt shaders here
	GpuAsset<Shader>& shader = GpuAssetManager->CompileShader("engine-data/spv/raytrace/pt/ptarray.shader");
	shader.onCompileRayTracing = [&]() {
		MakeRtPipeline();
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
	stages.push_back({ {}, vk::ShaderStageFlagBits::eRaygenKHR, *shader.rayGen.Lock().module, "main" });
	rg.setGeneralShader(static_cast<uint32>(stages.size() - 1));

	m_rtShaderGroups.push_back(rg);

	// Miss
	vk::RayTracingShaderGroupCreateInfoKHR mg{};
	mg.setType(vk::RayTracingShaderGroupTypeKHR::eGeneral) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	stages.push_back({ {}, vk::ShaderStageFlagBits::eMissKHR, *shader.miss.Lock().module, "main" });
	mg.setGeneralShader(static_cast<uint32>(stages.size() - 1));

	m_rtShaderGroups.push_back(mg);

	vk::RayTracingShaderGroupCreateInfoKHR hg{};
	hg.setType(vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	stages.push_back({ {}, vk::ShaderStageFlagBits::eClosestHitKHR, *shader.closestHit.Lock().module, "main" });
	hg.setClosestHitShader(static_cast<uint32>(stages.size() - 1));

	m_rtShaderGroups.push_back(hg);


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

	m_pipelineLayout = Device->createPipelineLayoutUnique(pipelineLayoutInfo);

	// Assemble the shader stages and recursion depth info into the ray tracing pipeline
	vk::RayTracingPipelineCreateInfoKHR rayPipelineInfo{};
	rayPipelineInfo
		// Stages are shaders
		.setStages(stages);

	rayPipelineInfo
		// 1-raygen, n-miss, n-(hit[+anyhit+intersect])
		.setGroups(m_rtShaderGroups)
		// Note that it is preferable to keep the recursion level as low as possible, replacing it by a loop formulation
		// instead.

		.setMaxRecursionDepth(10) // Ray depth TODO:
		.setLayout(m_pipelineLayout.get());
	m_pipeline = Device->createRayTracingPipelineKHRUnique({}, rayPipelineInfo);

	CreateRtShaderBindingTable();
}

void PathtracedCubemapArray::CreateRtShaderBindingTable()
{
	auto groupCount = static_cast<uint32>(m_rtShaderGroups.size());     // 3 shaders: raygen, miss, chit
	uint32 groupHandleSize = Device->pd.rtProps.shaderGroupHandleSize;  // Size of a program identifier
	uint32 baseAlignment = Device->pd.rtProps.shaderGroupBaseAlignment; // Size of shader alignment

	// Fetch all the shader handles used in the pipeline, so that they can be written in the SBT
	uint32 sbtSize = groupCount * baseAlignment;

	std::vector<byte> shaderHandleStorage(sbtSize);
	Device->getRayTracingShaderGroupHandlesKHR(m_pipeline.get(), 0, groupCount, sbtSize, shaderHandleStorage.data());
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
}
} // namespace vl
