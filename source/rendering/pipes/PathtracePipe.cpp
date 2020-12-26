#include "PathtracePipe.h"

#include "engine/console/ConsoleVariable.h"
#include "rendering/Renderer.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/SceneCamera.h"


ConsoleVariable<int32> cons_pathtraceBounces{ "r.pathtrace.bounces", 1, "Set pathtrace bounces" };

namespace {
struct PushConstant {
	int32 bounces;
	int32 frame;
	int32 pointlightCount;
	int32 spotlightCount;
	int32 dirlightCount;
	int32 quadlightCount;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
vk::UniquePipelineLayout PathtracePipe::MakePipelineLayout()
{
	std::array layouts{
		Layouts->doubleStorageImage.handle(),          // images
		Layouts->singleUboDescLayout.handle(),         // camera
		Layouts->accelLayout.handle(),                 // as
		Layouts->bufferAndSamplersDescLayout.handle(), // geometry and texture
		Layouts->singleStorageBuffer.handle(),         // pointlights
		Layouts->bufferAndSamplersDescLayout.handle(), // spotlights
		Layouts->bufferAndSamplersDescLayout.handle(), // dirlights
		Layouts->singleStorageBuffer.handle(),         // quadlights
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

vk::UniquePipeline PathtracePipe::MakePipeline()
{
	// all rt shaders here
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/pathtrace/pathtrace.shader");
	gpuShader.onCompile = [&]() {
		StaticPipes::Recompile<PathtracePipe>();
	};

	GpuAsset<Shader>& gpuShader2
		= GpuAssetManager->CompileShader("engine-data/spv/pathtrace/pathtrace-quadlight.shader");
	gpuShader2.onCompileRayTracing = [&]() {
		StaticPipes::Recompile<PathtracePipe>();
	};

	GpuAsset<Shader>& gpuShader3 = GpuAssetManager->CompileShader("engine-data/spv/pathtrace/pathtrace-shadow.shader");
	gpuShader3.onCompileRayTracing = [&]() {
		StaticPipes::Recompile<PathtracePipe>();
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
	vk::RayTracingShaderGroupCreateInfoKHR mg{};           // miss general 0
	mg.setType(vk::RayTracingShaderGroupTypeKHR::eGeneral) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	stages.push_back({ {}, vk::ShaderStageFlagBits::eMissKHR, *gpuShader.miss.Lock().module, "main" });
	mg.setGeneralShader(static_cast<uint32>(stages.size() - 1));

	m_rtShaderGroups.push_back(mg);

	// Miss
	vk::RayTracingShaderGroupCreateInfoKHR mg1{};           // miss shadow 1
	mg1.setType(vk::RayTracingShaderGroupTypeKHR::eGeneral) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	stages.push_back({ {}, vk::ShaderStageFlagBits::eMissKHR, *gpuShader3.miss.Lock().module, "main" });
	mg1.setGeneralShader(static_cast<uint32>(stages.size() - 1));

	m_rtShaderGroups.push_back(mg1);

	vk::RayTracingShaderGroupCreateInfoKHR hg{};                     // gltf mat offset 0
	hg.setType(vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	stages.push_back({ {}, vk::ShaderStageFlagBits::eClosestHitKHR, *gpuShader.closestHit.Lock().module, "main" });
	hg.setClosestHitShader(static_cast<uint32>(stages.size() - 1));
	stages.push_back({ {}, vk::ShaderStageFlagBits::eAnyHitKHR, *gpuShader.anyHit.Lock().module, "main" }); // for mask
	hg.setAnyHitShader(static_cast<uint32>(stages.size() - 1));

	m_rtShaderGroups.push_back(hg);

	vk::RayTracingShaderGroupCreateInfoKHR hg2{};                     // quad lights offest 1
	hg2.setType(vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	stages.push_back({ {}, vk::ShaderStageFlagBits::eClosestHitKHR, *gpuShader2.closestHit.Lock().module, "main" });
	hg2.setClosestHitShader(static_cast<uint32>(stages.size() - 1));

	m_rtShaderGroups.push_back(hg2);

	vk::RayTracingShaderGroupCreateInfoKHR hg3{};                     // shadow offset 2
	hg3.setType(vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	stages.push_back({ {}, vk::ShaderStageFlagBits::eClosestHitKHR, *gpuShader3.closestHit.Lock().module, "main" });
	hg3.setClosestHitShader(static_cast<uint32>(stages.size() - 1));
	stages.push_back({ {}, vk::ShaderStageFlagBits::eAnyHitKHR, *gpuShader3.anyHit.Lock().module, "main" }); // for mask
	hg3.setAnyHitShader(static_cast<uint32>(stages.size() - 1));

	m_rtShaderGroups.push_back(hg3);


	// Assemble the shader stages and recursion depth info into the ray tracing pipeline
	vk::RayTracingPipelineCreateInfoKHR rayPipelineInfo{};
	rayPipelineInfo
		// Stages are shaders
		.setStages(stages)
		// 1-raygen, n-miss, n-(hit[+anyhit+intersect])
		.setGroups(m_rtShaderGroups)
		.setMaxPipelineRayRecursionDepth(1)
		.setLayout(layout());


	auto pipeline = Device->createRayTracingPipelineKHRUnique({}, {}, rayPipelineInfo);

	auto groupCount = static_cast<uint32>(m_rtShaderGroups.size());
	uint32 groupHandleSize = Device->pd.raytracingProperties.shaderGroupHandleSize;  // Size of a program identifier
	uint32 baseAlignment = Device->pd.raytracingProperties.shaderGroupBaseAlignment; // Size of shader alignment

	// Fetch all the shader handles used in the pipeline, so that they can be written in the SBT
	uint32 sbtSize = groupCount * baseAlignment;

	std::vector<byte> shaderHandleStorage(sbtSize);
	Device->getRayTracingShaderGroupHandlesKHR(
		pipeline.value.get(), 0, groupCount, sbtSize, shaderHandleStorage.data());
	// Write the handles in the SBT
	m_rtSBTBuffer = RBuffer{ sbtSize,
		vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderBindingTableKHR
			| vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		vk::MemoryAllocateFlagBits::eDeviceAddress };

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

void PathtracePipe::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc,
	vk::DescriptorSet storageImagesDescSet, const vk::Extent3D& extent, int32 frame) const
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, pipeline());

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eRayTracingKHR, layout(), 0u, 1u, &storageImagesDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 1u, 1u,
		&sceneDesc.viewer.uboDescSet[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eRayTracingKHR, layout(), 2u, 1u, &sceneDesc.scene->sceneAsDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 3u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSetGeometryAndTextures[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 4u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSetPointlights[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 5u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSetSpotlights[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 6u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSetDirlights[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 7u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSetQuadlights[sceneDesc.frameIndex], 0u, nullptr);

	vk::DeviceSize progSize = Device->pd.raytracingProperties.shaderGroupBaseAlignment; // Size of a program identifier

	// Miss index
	vk::DeviceSize missOffset = 1u * progSize;

	// Hit index
	vk::DeviceSize hitGroupOffset = 3u * progSize;

	// TODO: tidy
	const vk::StridedDeviceAddressRegionKHR raygenShaderBindingTable{ m_rtSBTBuffer.address(), progSize, progSize };
	const vk::StridedDeviceAddressRegionKHR missShaderBindingTable{ m_rtSBTBuffer.address() + missOffset, progSize,
		progSize };
	const vk::StridedDeviceAddressRegionKHR hitShaderBindingTable{ m_rtSBTBuffer.address() + hitGroupOffset, progSize,
		progSize };
	const vk::StridedDeviceAddressRegionKHR callableShaderBindingTable;

	PushConstant pc{
		std::max(*cons_pathtraceBounces, 0),
		frame,
		sceneDesc.scene->tlas.sceneDesc.pointlightCount,
		sceneDesc.scene->tlas.sceneDesc.spotlightCount,
		sceneDesc.scene->tlas.sceneDesc.dirlightCount,
		sceneDesc.scene->tlas.sceneDesc.quadlightCount,
	};

	cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR, 0u,
		sizeof(PushConstant), &pc);

	cmdBuffer.traceRaysKHR(&raygenShaderBindingTable, &missShaderBindingTable, &hitShaderBindingTable,
		&callableShaderBindingTable, extent.width, extent.height, 1);
}
} // namespace vl
