#include "RaytraceMirrorReflections.h"

#include "rendering/pipes/MirrorPipe.h"
#include "rendering/pipes/PathtraceCubemapArrayPipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneIrragrid.h"
#include "rendering/util/WriteDescriptorSets.h"

namespace vl {
RaytraceMirrorReflections::RaytraceMirrorReflections()
{
	for (size_t i = 0; i < c_framesInFlight; i++) {
		descSet[i] = Layouts->singleStorageImage.AllocDescriptorSet();
		DEBUG_NAME_AUTO(descSet[i]);
	}
}

void RaytraceMirrorReflections::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const
{
	result[sceneDesc.frameIndex].TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eFragmentShader,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	StaticPipes::Get<MirrorPipe>().Draw(cmdBuffer, sceneDesc);

	result[sceneDesc.frameIndex].TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
		vk::PipelineStageFlagBits::eFragmentShader);
}

void RaytraceMirrorReflections::Resize(vk::Extent2D extent) const
{
	for (int32 i = 0; i < c_framesInFlight; ++i) {
		result[i] = RImage2D("MirrorBuffer",
			vk::Extent2D{ static_cast<uint32>(extent.width), static_cast<uint32>(extent.height) },
			vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

		rvk::writeDescriptorImages(
			descSet[i], 0u, { result[i].view() }, vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);
	}
}
} // namespace vl
