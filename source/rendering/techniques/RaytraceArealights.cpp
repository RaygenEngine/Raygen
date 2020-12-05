#include "RaytraceArealights.h"

#include "engine/console/ConsoleVariable.h"
#include "rendering/pipes/ArealightsPipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneIrragrid.h"
#include "rendering/util/WriteDescriptorSets.h"

// TODO: use specific for each technique instance and waitIdle() resize
ConsoleVariable<float> cons_arealightsScale{ "r.arealights.scale", 1.f, "Set arealights scale" };

namespace vl {
RaytraceArealights::RaytraceArealights()
{
	for (size_t i = 0; i < c_framesInFlight; i++) {
		descSet[i] = Layouts->singleStorageImage.AllocDescriptorSet();
		DEBUG_NAME(descSet[i], "area lights storage image");
	}
}

void RaytraceArealights::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const
{
	result[sceneDesc.frameIndex].TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eFragmentShader,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	auto extent = result[sceneDesc.frameIndex].extent;

	StaticPipes::Get<ArealightsPipe>().Draw(cmdBuffer, sceneDesc, descSet[sceneDesc.frameIndex], extent);

	result[sceneDesc.frameIndex].TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
		vk::PipelineStageFlagBits::eFragmentShader);
}

void RaytraceArealights::Resize(vk::Extent2D extent)
{
	for (int32 i = 0; i < c_framesInFlight; ++i) {
		result[i] = RImage2D("ArealightBuffer",
			vk::Extent2D{ static_cast<uint32>(extent.width * cons_arealightsScale),
				static_cast<uint32>(extent.height * cons_arealightsScale) },
			vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

		rvk::writeDescriptorImages(
			descSet[i], 0u, { result[i].view() }, vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);
	}
}
} // namespace vl
