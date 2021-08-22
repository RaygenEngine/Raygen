#include "RaytraceMirrorReflections.h"

#include "engine/console/ConsoleVariable.h"
#include "rendering/pipes/MirrorPipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneIrragrid.h"

// TODO: use specific for each technique instance and waitIdle() resize
ConsoleVariable<float> cons_mirrorScale{ "r.mirror.scale", 1.f, "Set the scale of the mirror texture." };

namespace vl {
RaytraceMirrorReflections::RaytraceMirrorReflections()
{
	for (size_t i = 0; i < c_framesInFlight; i++) {
		descSet[i] = Layouts->singleStorageImage.AllocDescriptorSet();
		DEBUG_NAME(descSet[i], "mirror storage image");
	}
}

void RaytraceMirrorReflections::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const
{
	result[sceneDesc.frameIndex].TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eFragmentShader,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	auto extent = result[sceneDesc.frameIndex].extent;

	StaticPipes::Get<MirrorPipe>().Draw(cmdBuffer, sceneDesc, descSet[sceneDesc.frameIndex], extent);

	result[sceneDesc.frameIndex].TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
		vk::PipelineStageFlagBits::eFragmentShader);
}

void RaytraceMirrorReflections::Resize(vk::Extent2D extent)
{
	for (int32 i = 0; i < c_framesInFlight; ++i) {
		result[i] = RImage2D("MirrorBuffer",
			vk::Extent2D{ static_cast<uint32>(extent.width * cons_mirrorScale),
				static_cast<uint32>(extent.height * cons_mirrorScale) },
			vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

		rvk::writeDescriptorImages(
			descSet[i], 0u, { result[i].view() }, vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);
	}
}
} // namespace vl
