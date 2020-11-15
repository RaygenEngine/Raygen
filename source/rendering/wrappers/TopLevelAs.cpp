#include "TopLevelAs.h"

#include "assets/StdAssets.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuCubemap.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/assets/GpuMaterialInstance.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneDirlight.h"
#include "rendering/scene/SceneGeometry.h"
#include "rendering/scene/ScenePointlight.h"
#include "rendering/scene/SceneReflProbe.h"
#include "rendering/scene/SceneSpotlight.h"
#include "rendering/wrappers/CmdBuffer.h"

#include <glm/gtc/type_ptr.hpp>

namespace {
vk::AccelerationStructureInstanceKHR AsInstanceToVkGeometryInstanceKHR(const vl::AsInstance& instance)
{
	vk::AccelerationStructureInstanceKHR gInst{};
	// The matrices for the instance transforms are row-major, instead of
	// column-major in the rest of the application
	glm::mat4 transp = glm::transpose(instance.transform);
	// The gInst.transform value only contains 12 values, corresponding to a 4x3
	// matrix, hence saving the last row that is anyway always (0,0,0,1). Since
	// the matrix is row-major, we simply copy the first 12 values of the
	// original 4x4 matrix
	memcpy(&gInst.transform, glm::value_ptr(transp), sizeof(gInst.transform));
	gInst
		.setInstanceCustomIndex(instance.instanceId) //
		.setInstanceShaderBindingTableRecordOffset(instance.materialId)
		.setFlags(instance.flags)
		.setAccelerationStructureReference(instance.blas)
		.setMask(1); // Use a single mask for all objects for now. Mask must match the cullMask parameter of
					 // rayQueryInitializeEXT in the shader.

	// https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GLSL_EXT_ray_query.txt#L252

	return gInst;
}
} // namespace


namespace {

struct RtGeometryGroup {
	VkDeviceAddress vtxBuffer;
	VkDeviceAddress indBuffer;
	VkDeviceAddress materialUbo;

	int32 indexOffset;
	int32 primOffset;

	glm::mat4 transform;
	glm::mat4 invTransform;
};

std::vector<RtGeometryGroup> g_tempGeometryGroups;

} // namespace

namespace vl {
TopLevelAs::TopLevelAs(const std::vector<SceneGeometry*>& geoms, Scene* scene)
{
	sceneDesc.descSetSpotlights = Layouts->bufferAndSamplersDescLayout.AllocDescriptorSet(
		static_cast<int32>(scene->Get<SceneSpotlight>().size()));
	sceneDesc.descSetDirlights = Layouts->bufferAndSamplersDescLayout.AllocDescriptorSet(
		static_cast<int32>(scene->Get<SceneDirlight>().size()));

	sceneDesc.descSetPointlights = Layouts->singleStorageBuffer.AllocDescriptorSet();
	sceneDesc.descSetReflprobes = Layouts->bufferAndSamplersDescLayout.AllocDescriptorSet(
		static_cast<int32>(scene->Get<SceneReflprobe>().size()));

	int32 totalGroups = 0;


	for (auto geom : geoms) {
		auto transform = geom->transform;

		for (auto& gg : geom->mesh.Lock().geometryGroups) {
			if (gg.material.Lock().archetype != StdAssets::GltfArchetype()) {
				continue;
			}

			AsInstance inst{};
			inst.transform = transform;    // Position of the instance
			inst.instanceId = totalGroups; // gl_InstanceID
			inst.blas = Device->getAccelerationStructureAddressKHR(gg.blas.handle());
			inst.materialId = 0;
			inst.flags = vk::GeometryInstanceFlagBitsKHR::eTriangleFrontCounterclockwise;
			instances.emplace_back(AsInstanceToVkGeometryInstanceKHR(inst));

			sceneDesc.AddGeomGroup(gg, geom->mesh.Lock(), transform);
			totalGroups++;
		}
	}

	auto imgSize = GpuAssetManager->Z_GetSize();
	sceneDesc.descSet = Layouts->bufferAndSamplersDescLayout.AllocDescriptorSet(static_cast<int32>(imgSize));


	sceneDesc.WriteImages();

	sceneDesc.WritePointlights(scene->Get<ScenePointlight>().condensed);
	sceneDesc.WriteSpotlights(scene->Get<SceneSpotlight>().condensed);
	sceneDesc.WriteDirlights(scene->Get<SceneDirlight>().condensed);
	sceneDesc.WriteReflprobes(scene->Get<SceneReflprobe>().condensed);
	sceneDesc.WriteGeomGroups();
	Build();
	Device->waitIdle();
	g_tempGeometryGroups.clear();
}


void RtSceneDescriptor::AddGeomGroup(const GpuGeometryGroup& group, const GpuMesh& mesh, const glm::mat4& transform)
{
	auto& dstGeomGroup = g_tempGeometryGroups.emplace_back();

	dstGeomGroup.indBuffer = mesh.combinedIndexBuffer.address();
	dstGeomGroup.vtxBuffer = mesh.combinedVertexBuffer.address();
	dstGeomGroup.materialUbo = group.material.Lock().rtMaterialBuffer.address();

	dstGeomGroup.indexOffset = group.indexOffset;
	dstGeomGroup.primOffset = group.primOffset;

	dstGeomGroup.transform = transform;
	dstGeomGroup.invTransform = glm::inverse(transform);
}

void RtSceneDescriptor::WriteImages()
{
	std::vector<vk::DescriptorImageInfo> images;

	vk::DescriptorImageInfo viewInfoDefault;
	viewInfoDefault
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
		.setSampler(GpuAssetManager->GetDefaultSampler());

	auto defaultView = GpuAssetManager->GetGpuHandle<Image>({}).Lock().image.view();
	images.reserve(GpuAssetManager->Z_GetSize());

	for (int32 i = 0; auto asset : GpuAssetManager->Z_GetAssets()) {
		auto gpuImg = dynamic_cast<vl::GpuImage*>(asset);
		viewInfoDefault.setImageView(gpuImg ? gpuImg->image.view() : defaultView);
		images.emplace_back(viewInfoDefault);
	}


	for (int32 i = 0; i < c_framesInFlight; i++) {
		vk::WriteDescriptorSet imgWriteSet{};
		imgWriteSet
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler) //
			.setImageInfo(images)
			.setDstBinding(1u)
			.setDstSet(descSet[i])
			.setDstArrayElement(0u);

		Device->updateDescriptorSets({ imgWriteSet }, nullptr);
	}
}


void RtSceneDescriptor::WriteSpotlights(const std::vector<SceneSpotlight*>& spotlights)
{
	spotlightCount = static_cast<int32>(spotlights.size());

	uint32 uboSize = sizeof(Spotlight_Ubo) + sizeof(Spotlight_Ubo) % 8;

	spotlightsBuffer = { (uboSize * spotlightCount),
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer
			| vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		vk::MemoryAllocateFlagBits::eDeviceAddress };

	DEBUG_NAME_AUTO(spotlightsBuffer);

	byte* mapCursor = reinterpret_cast<byte*>(Device->mapMemory(spotlightsBuffer.memory(), 0, spotlightsBuffer.size));

	for (auto spotlight : spotlights) {
		memcpy(mapCursor, &spotlight->ubo, sizeof(Spotlight_Ubo));
		mapCursor += uboSize;
	}

	Device->unmapMemory(spotlightsBuffer.memory());


	vk::DescriptorBufferInfo bufInfo{};
	bufInfo
		.setOffset(0) //
		.setRange(VK_WHOLE_SIZE)
		.setBuffer(spotlightsBuffer.handle());


	for (int32 i = 0; i < c_framesInFlight; ++i) {
		vk::WriteDescriptorSet bufWriteSet{};
		bufWriteSet
			.setDescriptorType(vk::DescriptorType::eStorageBuffer) //
			.setDstBinding(0u)
			.setDstSet(descSetSpotlights[i])
			.setDstArrayElement(0u)
			.setBufferInfo(bufInfo);

		Device->updateDescriptorSets({ bufWriteSet }, nullptr);
	}


	if (spotlightCount == 0) {
		return;
	}

	for (int32 i = 0; i < c_framesInFlight; ++i) {
		std::vector<vk::DescriptorImageInfo> depthImages;
		vk::DescriptorImageInfo viewInfoDefault;

		viewInfoDefault.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal); //

		for (auto spotlight : spotlights) {
			viewInfoDefault //
				.setImageView(spotlight->shadowmapPass[i].framebuffer[0].view())
				.setSampler(spotlight->depthSampler);

			depthImages.emplace_back(viewInfoDefault);
		}

		vk::WriteDescriptorSet depthWrite{};
		depthWrite
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler) //
			.setImageInfo(depthImages)
			.setDstBinding(1u)
			.setDstSet(descSetSpotlights[i])
			.setDstArrayElement(0u);
		Device->updateDescriptorSets({ depthWrite }, nullptr);
	}
}

void RtSceneDescriptor::WriteDirlights(const std::vector<SceneDirlight*>& dirlights)
{
	dirlightCount = static_cast<int32>(dirlights.size());

	uint32 uboSize = sizeof(Dirlight_Ubo) + sizeof(Dirlight_Ubo) % 8;

	dirlightsBuffer = { (uboSize * dirlightCount),
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer
			| vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		vk::MemoryAllocateFlagBits::eDeviceAddress };

	DEBUG_NAME_AUTO(dirlightsBuffer);

	byte* mapCursor = reinterpret_cast<byte*>(Device->mapMemory(dirlightsBuffer.memory(), 0, dirlightsBuffer.size));

	for (auto dirlight : dirlights) {
		memcpy(mapCursor, &dirlight->ubo, sizeof(Dirlight_Ubo));
		mapCursor += uboSize;
	}

	Device->unmapMemory(dirlightsBuffer.memory());


	vk::DescriptorBufferInfo bufInfo{};
	bufInfo
		.setOffset(0) //
		.setRange(VK_WHOLE_SIZE)
		.setBuffer(dirlightsBuffer.handle());


	for (int32 i = 0; i < c_framesInFlight; ++i) {
		vk::WriteDescriptorSet bufWriteSet{};
		bufWriteSet
			.setDescriptorType(vk::DescriptorType::eStorageBuffer) //
			.setDstBinding(0u)
			.setDstSet(descSetDirlights[i])
			.setDstArrayElement(0u)
			.setBufferInfo(bufInfo);

		Device->updateDescriptorSets({ bufWriteSet }, nullptr);
	}


	if (dirlightCount == 0) {
		return;
	}

	for (int32 i = 0; i < c_framesInFlight; ++i) {
		std::vector<vk::DescriptorImageInfo> depthImages;
		vk::DescriptorImageInfo viewInfoDefault;

		viewInfoDefault.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal); //

		for (auto dirlight : dirlights) {
			viewInfoDefault //
				.setImageView(dirlight->shadowmapPass[i].framebuffer[0].view())
				.setSampler(dirlight->depthSampler);

			depthImages.emplace_back(viewInfoDefault);
		}

		vk::WriteDescriptorSet depthWrite{};
		depthWrite
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler) //
			.setImageInfo(depthImages)
			.setDstBinding(1u)
			.setDstSet(descSetDirlights[i])
			.setDstArrayElement(0u);
		Device->updateDescriptorSets({ depthWrite }, nullptr);
	}
}

void RtSceneDescriptor::WritePointlights(const std::vector<ScenePointlight*>& pointlights)
{
	pointlightCount = static_cast<int32>(pointlights.size());

	uint32 uboSize = sizeof(Pointlight_Ubo) + sizeof(Pointlight_Ubo) % 8;

	pointlightsBuffer = { (uboSize * pointlightCount),
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer
			| vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		vk::MemoryAllocateFlagBits::eDeviceAddress };

	DEBUG_NAME_AUTO(pointlightsBuffer);

	byte* mapCursor = reinterpret_cast<byte*>(Device->mapMemory(pointlightsBuffer.memory(), 0, pointlightsBuffer.size));

	for (auto pointlight : pointlights) {
		memcpy(mapCursor, &pointlight->ubo, sizeof(Pointlight_Ubo));
		mapCursor += uboSize;
	}

	Device->unmapMemory(pointlightsBuffer.memory());


	vk::DescriptorBufferInfo bufInfo{};
	bufInfo
		.setOffset(0) //
		.setRange(VK_WHOLE_SIZE)
		.setBuffer(pointlightsBuffer.handle());


	for (int32 i = 0; i < c_framesInFlight; ++i) {
		vk::WriteDescriptorSet bufWriteSet{};
		bufWriteSet
			.setDescriptorType(vk::DescriptorType::eStorageBuffer) //
			.setDstBinding(0u)
			.setDstSet(descSetPointlights[i])
			.setDstArrayElement(0u)
			.setBufferInfo(bufInfo);

		DEBUG_NAME_AUTO(descSetPointlights[i]);

		Device->updateDescriptorSets({ bufWriteSet }, nullptr);
	}
}

void RtSceneDescriptor::WriteReflprobes(const std::vector<SceneReflprobe*>& reflprobes)
{
	reflprobeCount = static_cast<int32>(reflprobes.size());

	uint32 uboSize = sizeof(Reflprobe_UBO) + sizeof(Reflprobe_UBO) % 8;

	reflprobesBuffer = { (uboSize * reflprobeCount),
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer
			| vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		vk::MemoryAllocateFlagBits::eDeviceAddress };

	DEBUG_NAME_AUTO(reflprobesBuffer);


	byte* mapCursor = reinterpret_cast<byte*>(Device->mapMemory(reflprobesBuffer.memory(), 0, reflprobesBuffer.size));

	for (auto reflprobe : reflprobes) {
		memcpy(mapCursor, &reflprobe->ubo, sizeof(Reflprobe_UBO));
		mapCursor += uboSize;
	}

	Device->unmapMemory(reflprobesBuffer.memory());


	vk::DescriptorBufferInfo bufInfo{};
	bufInfo
		.setOffset(0) //
		.setRange(VK_WHOLE_SIZE)
		.setBuffer(reflprobesBuffer.handle());


	for (int32 i = 0; i < c_framesInFlight; ++i) {
		vk::WriteDescriptorSet bufWriteSet{};
		bufWriteSet
			.setDescriptorType(vk::DescriptorType::eStorageBuffer) //
			.setDstBinding(0u)
			.setDstSet(descSetReflprobes[i])
			.setDstArrayElement(0u)
			.setBufferInfo(bufInfo);

		Device->updateDescriptorSets({ bufWriteSet }, nullptr);
	}


	auto quadSampler = GpuAssetManager->GetDefaultSampler();

	for (int32 i = 0; i < c_framesInFlight; ++i) {
		std::vector<vk::DescriptorImageInfo> cubeImages;
		vk::DescriptorImageInfo viewInfoDefault;

		viewInfoDefault
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
			.setSampler(quadSampler);

		for (auto reflprobe : reflprobes) {
			viewInfoDefault.setImageView(reflprobe->irradiance.view());
			cubeImages.emplace_back(viewInfoDefault);
			viewInfoDefault.setImageView(reflprobe->prefiltered.view());
			cubeImages.emplace_back(viewInfoDefault);
		}

		if (reflprobes.empty()) {
			auto view = GpuAssetManager->GetGpuHandle(PodHandle<Cubemap>()).Lock().cubemap.view();
			viewInfoDefault.setImageView(view);
			cubeImages.emplace_back(viewInfoDefault);
			viewInfoDefault.setImageView(view);
			cubeImages.emplace_back(viewInfoDefault);
		}
		DEBUG_NAME_AUTO(descSetReflprobes[i]);


		vk::WriteDescriptorSet depthWrite{};
		depthWrite
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler) //
			.setImageInfo(cubeImages)
			.setDstBinding(1u)
			.setDstSet(descSetReflprobes[i])
			.setDstArrayElement(0u);
		Device->updateDescriptorSets(depthWrite, nullptr);
	}
}

void RtSceneDescriptor::WriteGeomGroups()
{
	auto& geomGroupsVec = g_tempGeometryGroups;

	geomGroupsBuffer = RBuffer{ sizeof(RtGeometryGroup) * geomGroupsVec.size(), vk::BufferUsageFlagBits::eStorageBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };


	geomGroupsBuffer.UploadData(geomGroupsVec.data(), geomGroupsBuffer.size);

	vk::DescriptorBufferInfo sceneBufInfo{};
	sceneBufInfo
		.setOffset(0) //
		.setRange(VK_WHOLE_SIZE)
		.setBuffer(geomGroupsBuffer.handle());

	for (int32 i = 0; i < c_framesInFlight; ++i) {
		vk::WriteDescriptorSet bufWrite{};
		bufWrite
			.setDescriptorType(vk::DescriptorType::eStorageBuffer) //
			.setDstBinding(0)
			.setDstSet(descSet[i])
			.setDstArrayElement(0u)
			.setBufferInfo(sceneBufInfo);

		Device->updateDescriptorSets({ bufWrite }, nullptr);
	}
}


void TopLevelAs::AddAsInstance(AsInstance&& instance)
{
	instances.emplace_back(AsInstanceToVkGeometryInstanceKHR(instance));
}

void TopLevelAs::Clear()
{
	instances.clear();
}

void TopLevelAs::Build()
{
	vk::DeviceSize instanceDescsSizeInBytes = instances.size() * sizeof(VkAccelerationStructureInstanceKHR);

	RBuffer stagingbuffer{ instanceDescsSizeInBytes, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	// copy data to buffer
	stagingbuffer.UploadData(instances.data(), instanceDescsSizeInBytes);

	instanceBuffer = { instanceDescsSizeInBytes,
		vk::BufferUsageFlagBits::eRayTracingKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress
			| vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryAllocateFlagBits::eDeviceAddress };

	instanceBuffer.CopyBuffer(stagingbuffer);

	auto instanceAddress = Device->getBufferAddress(instanceBuffer.handle());

	DEBUG_NAME(instanceBuffer, "TLASInstances");

	vk::AccelerationStructureGeometryDataKHR geometry{};
	geometry.instances.setArrayOfPointers(VK_FALSE);
	geometry.instances.data.setDeviceAddress(instanceAddress);

	vk::AccelerationStructureGeometryKHR asGeoms{};
	asGeoms
		.setGeometryType(vk::GeometryTypeKHR::eInstances) //
		.setGeometry(geometry);

	vk::AccelerationStructureCreateGeometryTypeInfoKHR geometryCreate{};
	geometryCreate
		.setGeometryType(vk::GeometryTypeKHR::eInstances) //
		.setMaxPrimitiveCount(static_cast<uint32>(instances.size()))
		.setAllowsTransforms(VK_TRUE);

	vk::AccelerationStructureCreateInfoKHR asCreateInfo{};
	asCreateInfo
		.setType(vk::AccelerationStructureTypeKHR::eTopLevel) //
		.setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
		.setGeometryInfos(geometryCreate);


	uHandle = Device->createAccelerationStructureKHRUnique(asCreateInfo);

	DEBUG_NAME(uHandle, "Scene Tlas");

	AllocateMemory();

	// Compute the amount of scratch memory required by the acceleration structure builder
	vk::AccelerationStructureMemoryRequirementsInfoKHR asMemReqsInfo{};
	asMemReqsInfo
		.setType(vk::AccelerationStructureMemoryRequirementsTypeKHR::eBuildScratch) //
		.setBuildType(vk::AccelerationStructureBuildTypeKHR::eDevice)
		.setAccelerationStructure(uHandle.get());

	auto memReqs = Device->getAccelerationStructureMemoryRequirementsKHR(asMemReqsInfo);

	// Acceleration structure build requires some scratch space to store temporary information
	const auto scratchBufferSize = memReqs.memoryRequirements.size;

	// TODO: use a single scratch buffer based on the maximum requirements of the scene BVH
	scratchBuffer
		= { scratchBufferSize, vk::BufferUsageFlagBits::eRayTracingKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
			  vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryAllocateFlagBits::eDeviceAddress };

	auto scratchAddress = Device->getBufferAddress(scratchBuffer.handle());

	vk::AccelerationStructureGeometryKHR* pGeometry = &asGeoms;
	vk::AccelerationStructureBuildGeometryInfoKHR asBuildGeomInfo{};
	asBuildGeomInfo
		.setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace) //
		.setUpdate(VK_FALSE)
		.setSrcAccelerationStructure({})
		.setDstAccelerationStructure(uHandle.get())
		.setGeometryCount(1u)
		.setGeometryArrayOfPointers(VK_FALSE)
		.setType(vk::AccelerationStructureTypeKHR::eTopLevel)
		.setPpGeometries(&pGeometry)
		.setScratchData(scratchAddress);

	// Build Offsets info: n instances
	vk::AccelerationStructureBuildOffsetInfoKHR buildOffsetInfo{};
	buildOffsetInfo
		.setFirstVertex(0) //
		.setPrimitiveCount(static_cast<uint32_t>(instances.size()))
		.setPrimitiveOffset(0)
		.setTransformOffset(0);

	std::vector<const vk::AccelerationStructureBuildOffsetInfoKHR*> pBuildOffset;
	pBuildOffset.push_back(&buildOffsetInfo);

	{
		ScopedOneTimeSubmitCmdBuffer<Compute> cmdBuffer;

		// Make sure the copy of the instance buffer are copied before triggering the
		// acceleration structure build
		vk::MemoryBarrier barrier{};
		barrier
			.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite) //
			.setDstAccessMask(vk::AccessFlagBits::eAccelerationStructureWriteKHR);

		cmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR, vk::DependencyFlags{ 0 }, barrier, {}, {});

		cmdBuffer.buildAccelerationStructureKHR(asBuildGeomInfo, pBuildOffset);
	}
}
} // namespace vl
