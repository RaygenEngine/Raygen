#include "ProgressivePathtrace.h"

#include "engine/console/ConsoleVariable.h"
#include "rendering/pipes/PathtracePipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneIrragrid.h"
#include "rendering/util/WriteDescriptorSets.h"

// TODO: use specific for each technique instance and waitIdle() resize
ConsoleVariable<float> cons_pathtraceScale{ "r.pathtrace.scale", 1.f, "Set pathtrace scale" };

namespace vl {
ProgressivePathtrace::ProgressivePathtrace()
{
	for (size_t i = 0; i < c_framesInFlight; i++) {
		descSet[i] = Layouts->doubleStorageImage.AllocDescriptorSet();
		DEBUG_NAME(descSet[i], "area lights storage images");
	}
}

void ProgressivePathtrace::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	result[sceneDesc.frameIndex].TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eFragmentShader,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	auto extent = result[sceneDesc.frameIndex].extent;

	StaticPipes::Get<PathtracePipe>().Draw(cmdBuffer, sceneDesc, descSet[sceneDesc.frameIndex], extent, frame++);

	result[sceneDesc.frameIndex].TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
		vk::PipelineStageFlagBits::eFragmentShader);
}

void ProgressivePathtrace::Resize(vk::Extent2D extent)
{
	progressive = RImage2D("PathtraceProg",
		vk::Extent2D{ static_cast<uint32>(extent.width * cons_pathtraceScale),
			static_cast<uint32>(extent.height * cons_pathtraceScale) },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eGeneral);

	for (int32 i = 0; i < c_framesInFlight; ++i) {
		result[i] = RImage2D("PathtraceBuffer",
			vk::Extent2D{ static_cast<uint32>(extent.width * cons_pathtraceScale),
				static_cast<uint32>(extent.height * cons_pathtraceScale) },
			vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

		rvk::writeDescriptorImages(descSet[i], 0u, { result[i].view(), progressive.view() },
			vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);
	}
}
} // namespace vl
