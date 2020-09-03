#include "pch.h"
#include "TopLevelAs.h"

#include "assets/StdAssets.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/assets/GpuMaterialInstance.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/Device.h"
#include "rendering/scene/SceneGeometry.h"
#include "rendering/scene/SceneSpotlight.h"
#include "rendering/scene/Scene.h"

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

namespace vl {
TopLevelAs::TopLevelAs(const std::vector<SceneGeometry*>& geoms, Scene* scene)
{
	sceneDesc.descSet = Layouts->rtSceneDescLayout.AllocDescriptorSet();

	std::vector<vk::DescriptorImageInfo> descImages;
	vk::DescriptorImageInfo viewInfoDefault;

	viewInfoDefault
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
		.setSampler(GpuAssetManager->GetDefaultSampler());


	for (int i = 0, j = 0; i < static_cast<int>(geoms.size()); i++) {
		if (geoms[i] && geoms[i]->mesh.Lock().geometryGroups.size()) {
			for (auto& gg : geoms[i]->mesh.Lock().geometryGroups) {
				AsInstance inst{};
				inst.transform = geoms[i]->transform; // Position of the instance
				inst.instanceId = j;                  // gl_InstanceID
				inst.blas = Device->getAccelerationStructureAddressKHR(gg.blas.handle());
				inst.materialId = 0; // We will use the same hit group for all
				inst.flags = vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable; // WIP:
				instances.emplace_back(AsInstanceToVkGeometryInstanceKHR(inst));
				++j;


				if (j <= 25 && gg.material.Lock().archetype == StdAssets::GltfArchetype.handle) {
					auto h = gg.material.Lock().podHandle;
					auto colorView
						= GpuAssetManager->GetGpuHandle(h.Lock()->descriptorSet.samplers2d[0]).Lock().image.view();
					auto normalView
						= GpuAssetManager->GetGpuHandle(h.Lock()->descriptorSet.samplers2d[3]).Lock().image.view();
					auto metalRoughView
						= GpuAssetManager->GetGpuHandle(h.Lock()->descriptorSet.samplers2d[1]).Lock().image.view();


					viewInfoDefault.setImageView(colorView);
					descImages.emplace_back(viewInfoDefault);

					viewInfoDefault.setImageView(normalView);
					descImages.emplace_back(viewInfoDefault);

					viewInfoDefault.setImageView(metalRoughView);
					descImages.emplace_back(viewInfoDefault);
				}
			}
		}
	}

	sceneDesc.WriteDescriptorMesh(geoms[0]->mesh.Lock());
	sceneDesc.WriteAlbedoImages(descImages);


	sceneDesc.WriteSpotlights(scene->spotlights.elements);


	Build();
	Device->waitIdle();
}

void RtSceneDescriptor::WriteDescriptorMesh(const GpuMesh& mesh)
{
	vk::DescriptorBufferInfo vtxBufInfo{};
	vtxBufInfo
		.setOffset(0) //
		.setRange(VK_WHOLE_SIZE)
		.setBuffer(mesh.combinedVertexBuffer.handle());

	vk::DescriptorBufferInfo indBufInfo{};
	indBufInfo
		.setOffset(0) //
		.setRange(VK_WHOLE_SIZE)
		.setBuffer(mesh.combinedIndexBuffer.handle());


	vk::DescriptorBufferInfo indOffsetBufInfo{};
	indOffsetBufInfo
		.setOffset(0) //
		.setRange(VK_WHOLE_SIZE)
		.setBuffer(mesh.indexOffsetBuffer.handle());

	vk::DescriptorBufferInfo primOffsetBufInfo{};
	primOffsetBufInfo
		.setOffset(0) //
		.setRange(VK_WHOLE_SIZE)
		.setBuffer(mesh.primitiveOffsetBuffer.handle());

	for (int32 i = 0; i < c_framesInFlight; i++) {


		vk::WriteDescriptorSet bufWriteSet{};
		bufWriteSet
			.setDescriptorType(vk::DescriptorType::eStorageBuffer) //
			.setDstBinding(1)
			.setDstSet(descSet[i])
			.setDstArrayElement(0u)
			.setBufferInfo(vtxBufInfo);

		vk::WriteDescriptorSet bufWriteSet2{};
		bufWriteSet2
			.setDescriptorType(vk::DescriptorType::eStorageBuffer) //
			.setDstBinding(2)
			.setDstSet(descSet[i])
			.setDstArrayElement(0u)
			.setBufferInfo(indBufInfo);

		vk::WriteDescriptorSet bufWriteSet3{};
		bufWriteSet3
			.setDescriptorType(vk::DescriptorType::eStorageBuffer) //
			.setDstBinding(3)
			.setDstSet(descSet[i])
			.setDstArrayElement(0u)
			.setBufferInfo(indOffsetBufInfo);

		vk::WriteDescriptorSet bufWriteSet4{};
		bufWriteSet4
			.setDescriptorType(vk::DescriptorType::eStorageBuffer) //
			.setDstBinding(4)
			.setDstSet(descSet[i])
			.setDstArrayElement(0u)
			.setBufferInfo(primOffsetBufInfo);


		Device->updateDescriptorSets({ bufWriteSet, bufWriteSet2, bufWriteSet3, bufWriteSet4 }, nullptr);
	}
}
void RtSceneDescriptor::WriteAlbedoImages(const std::vector<vk::DescriptorImageInfo>& images)
{
	for (int32 i = 0; i < c_framesInFlight; i++) {
		vk::WriteDescriptorSet imgWriteSet{};
		imgWriteSet
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler) //
			.setImageInfo(images)
			.setDstBinding(0u)
			.setDstSet(descSet[i])
			.setDstArrayElement(0u);

		Device->updateDescriptorSets({ imgWriteSet }, nullptr);
	}
}


void RtSceneDescriptor::WriteSpotlights(const std::vector<SceneSpotlight*>& spotlights)
{
	uint32 count = 0;
	for (auto spotlight : spotlights) {
		if (spotlight) {
			count++;
		}
	}

	spotlightsBuffer = { (sizeof(Spotlight_Ubo) * count),
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer
			| vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
		vk::MemoryAllocateFlagBits::eDeviceAddress };

	byte* mapCursor = reinterpret_cast<byte*>(Device->mapMemory(spotlightsBuffer.memory(), 0, spotlightsBuffer.size));

	for (int32 i = 0; auto spotlight : spotlights) {
		if (!spotlight) {
			continue;
		}
		memcpy(mapCursor, &spotlight->ubo, sizeof(Spotlight_Ubo));
		mapCursor += sizeof(Spotlight_Ubo);
		++i;
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
			.setDstBinding(5u)
			.setDstSet(descSet[i])
			.setDstArrayElement(0u)
			.setBufferInfo(bufInfo);

		Device->updateDescriptorSets({ bufWriteSet }, nullptr);
	}

	if (count == 0) {
		return;
	}

	for (int32 i = 0; i < c_framesInFlight; ++i) {
		std::vector<vk::DescriptorImageInfo> depthImages;
		vk::DescriptorImageInfo viewInfoDefault;

		viewInfoDefault.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal); //

		for (auto spotlight : spotlights) {
			if (!spotlight) {
				continue;
			}
			viewInfoDefault //
				.setImageView(spotlight->shadowmap[i].framebuffer[0].view())
				.setSampler(spotlight->shadowmap[i].depthSampler);

			depthImages.emplace_back(viewInfoDefault);
		}

		vk::WriteDescriptorSet depthWrite{};
		depthWrite
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler) //
			.setImageInfo(depthImages)
			.setDstBinding(6u)
			.setDstSet(descSet[i])
			.setDstArrayElement(0u);
		Device->updateDescriptorSets({ depthWrite }, nullptr);

		for (uint32 j = depthImages.size(); j < 16; ++j) {
			depthWrite
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler) //
				.setImageInfo(viewInfoDefault)
				.setDstBinding(6u)
				.setDstSet(descSet[i])
				.setDstArrayElement(j);
			Device->updateDescriptorSets({ depthWrite }, nullptr);
		}
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

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	Device->computeCmdBuffer.begin(beginInfo);

	// Make sure the copy of the instance buffer are copied before triggering the
	// acceleration structure build
	vk::MemoryBarrier barrier{};
	barrier
		.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite) //
		.setDstAccessMask(vk::AccessFlagBits::eAccelerationStructureWriteKHR);

	Device->computeCmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR, vk::DependencyFlags{ 0 }, barrier, {}, {});

	Device->computeCmdBuffer.buildAccelerationStructureKHR(asBuildGeomInfo, pBuildOffset);

	Device->computeCmdBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBuffers(Device->computeCmdBuffer);

	Device->computeQueue.submit(submitInfo, {});

	Device->computeQueue.waitIdle(); // NEXT:
}

} // namespace vl
