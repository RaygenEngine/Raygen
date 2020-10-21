#include "PathtracedCubemap.h"

#include "assets/AssetRegistry.h"
#include "assets/PodEditor.h"
#include "assets/pods/Cubemap.h"
#include "assets/pods/EnvironmentMap.h"
#include "engine/console/ConsoleVariable.h"
#include "engine/Input.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuCubemap.h"
#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/Layouts.h"
#include "rendering/Renderer.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/util/WriteDescriptorSets.h"
#include "rendering/scene/SceneReflProbe.h"

namespace {
struct PushConstant {
	glm::mat4 viewInverse;
	glm::mat4 projInverse;
	int32 pointlightCount;
	float innerRadius;
};

// static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
PathtracedCubemap::PathtracedCubemap(GpuEnvironmentMap* envmapAsset, glm::vec3 position, uint32 res)
	: m_envmapAsset(envmapAsset)
	, m_resolution(res)
{
	MakeRtPipeline();
	CreateMatrices(position);
	CreateFaceAttachments();

	for (int32 i = 0; i < 6; ++i) {
		m_faceDescSets[i] = Layouts->singleStorageImage.AllocDescriptorSet();

		rvk::writeDescriptorImages(m_faceDescSets[i], 0u, { m_faceAttachments[i].view() },
			vk::DescriptorType::eStorageImage, nullptr, vk::ImageLayout::eGeneral);

		DEBUG_NAME_AUTO(m_faceDescSets[i]); // WIP: bad
	}
} // namespace vl

void PathtracedCubemap::MakeRtPipeline()
{
	std::array layouts{
		Layouts->singleStorageImage.handle(),
		Layouts->accelLayout.handle(),
		Layouts->bufferAndSamplersDescLayout.handle(),
		Layouts->singleStorageBuffer.handle(),
	};

	// all rt shaders here
	GpuAsset<Shader>& shader = GpuAssetManager->CompileShader("engine-data/spv/raytrace/ptcube.shader");
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

void PathtracedCubemap::CreateRtShaderBindingTable()
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

void PathtracedCubemap::CreateMatrices(const glm::vec3& reflprobePos)
{
	// right
	m_viewMats[0] = glm::lookAt(reflprobePos, reflprobePos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	// left
	m_viewMats[1] = glm::lookAt(reflprobePos, reflprobePos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	// up
	m_viewMats[2] = glm::lookAt(reflprobePos, reflprobePos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
	// down
	m_viewMats[3] = glm::lookAt(reflprobePos, reflprobePos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
	// front
	m_viewMats[4] = glm::lookAt(reflprobePos, reflprobePos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0));
	// back
	m_viewMats[5] = glm::lookAt(reflprobePos, reflprobePos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));
}

void PathtracedCubemap::CreateFaceAttachments()
{
	for (int32 i = 0; i < 6; ++i) {
		m_faceAttachments[i] = RImageAttachment(m_resolution, m_resolution, vk::Format::eR32G32B32A32Sfloat,
			vk::ImageTiling::eOptimal, vk::ImageLayout::eUndefined,
			vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eDeviceLocal, "ptCubeFace" + i);

		m_faceAttachments[i].BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
			vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eRayTracingShaderKHR);
	}
}

void PathtracedCubemap::Calculate(vk::DescriptorSet sceneAsDescSet, vk::DescriptorSet sceneGeomDataDescSet,
	vk::DescriptorSet ScenePointlightDescSet, int32 pointlightCount, SceneReflprobe* rp)
{
	Device->waitIdle();

	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setCommandPool(Device->graphicsCmdPool.get())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1u);

	auto cmdBuffer = std::move(Device->allocateCommandBuffers(allocInfo)[0]);

	vk::CommandBufferBeginInfo beginInfo{};
	cmdBuffer.begin(beginInfo);
	{

		cmdBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, m_pipeline.get());


		DEBUG_NAME_AUTO(sceneAsDescSet);
		DEBUG_NAME_AUTO(sceneGeomDataDescSet);
		DEBUG_NAME_AUTO(ScenePointlightDescSet);


		cmdBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eRayTracingKHR, m_pipelineLayout.get(), 1u, 1u, &sceneAsDescSet, 0u, nullptr);

		cmdBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eRayTracingKHR, m_pipelineLayout.get(), 2u, 1u, &sceneGeomDataDescSet, 0u, nullptr);

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_pipelineLayout.get(), 3u, 1u,
			&ScenePointlightDescSet, 0u, nullptr);

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

		const vk::StridedBufferRegionKHR raygenShaderBindingTable
			= { m_rtSBTBuffer.handle(), rayGenOffset, progSize, sbtSize };
		const vk::StridedBufferRegionKHR missShaderBindingTable
			= { m_rtSBTBuffer.handle(), missOffset, progSize, sbtSize };
		const vk::StridedBufferRegionKHR hitShaderBindingTable
			= { m_rtSBTBuffer.handle(), hitGroupOffset, progSize, sbtSize };
		const vk::StridedBufferRegionKHR callableShaderBindingTable;


		auto projInverse = glm::perspective(glm::radians(90.0f), 1.f, 1.f, 25.f);
		projInverse[1][1] *= -1;
		projInverse = glm::inverse(projInverse);

		for (int32 i = 0; i < 6; ++i) {
			PushConstant pc{ glm::inverse(m_viewMats[i]), projInverse, pointlightCount, rp->ubo.innerRadius };

			cmdBuffer.pushConstants(m_pipelineLayout.get(),
				vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR, 0u, sizeof(PushConstant),
				&pc);

			cmdBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eRayTracingKHR, m_pipelineLayout.get(), 0u, 1u, &m_faceDescSets[i], 0u, nullptr);

			cmdBuffer.traceRaysKHR(&raygenShaderBindingTable, &missShaderBindingTable, &hitShaderBindingTable,
				&callableShaderBindingTable, m_resolution, m_resolution, 1);

			m_faceAttachments[i].TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
				vk::ImageLayout::eTransferSrcOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
				vk::PipelineStageFlagBits::eTransfer);
		}
	}
	cmdBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBuffers(cmdBuffer);

	Device->graphicsQueue.submit(submitInfo, {});

	Device->waitIdle();

	EditPods();
}

void PathtracedCubemap::EditPods()
{
	PodHandle<EnvironmentMap> envMap = m_envmapAsset->podHandle;

	if (envMap.IsDefault()) {
		return;
	}

	PodHandle<Cubemap> ptcube = envMap.Lock()->skybox;

	if (ptcube.IsDefault()) {
		PodEditor e(envMap);
		auto& [entry, irr] = AssetRegistry::CreateEntry<Cubemap>("gen-data/generated/cubemap");

		e.pod->skybox = entry->GetHandleAs<Cubemap>();
		ptcube = entry->GetHandleAs<Cubemap>();
	}

	PodEditor cubemapEditor(ptcube);

	cubemapEditor->resolution = m_resolution;
	cubemapEditor->format = ImageFormat::Hdr;

	auto bytesPerPixel = cubemapEditor->format == ImageFormat::Hdr ? 4u * 4u : 4u;
	auto size = m_resolution * m_resolution * bytesPerPixel * 6;

	cubemapEditor->data.resize(size);


	for (uint32 i = 0; i < 6; ++i) {

		auto& img = m_faceAttachments[i];

		RBuffer stagingbuffer{ m_resolution * m_resolution * bytesPerPixel, vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

		img.CopyImageToBuffer(stagingbuffer);

		void* data = Device->mapMemory(stagingbuffer.memory(), 0, VK_WHOLE_SIZE, {});

		memcpy(cubemapEditor->data.data() + size * i / 6, data, m_resolution * m_resolution * bytesPerPixel);

		Device->unmapMemory(stagingbuffer.memory());
	}
}

} // namespace vl
