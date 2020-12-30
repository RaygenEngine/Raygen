#include "ProgressivePathtrace.h"

#include "engine/console/ConsoleVariable.h"
#include "rendering/pipes/BdptPipe.h"
#include "rendering/pipes/NaivePathtracePipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/pipes/StochasticPathtracePipe.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneIrragrid.h"
#include "rendering/util/WriteDescriptorSets.h"

namespace {
enum class PtMode
{
	Naive,
	Stochastic,
	Bdpt,
};
};

ConsoleVariable<float> cons_pathtraceScale{ "r.pathtrace.scale", 1.f, "Set pathtrace scale" };
ConsoleVariable<int32> cons_pathtraceBounces{ "r.pathtrace.bounces", 1, "Set pathtrace bounces" };
ConsoleVariable<PtMode> const_pathtraceMode{ "r.pathtrace.mode", PtMode::Naive,
	"Set pathtrace mode. Naive: unidirectional, not entirely naive, it just lacks direct light sampling,"
	"Stochastic: similar to naive but uses direct light and light MIS,"
	"Bpdt: Bidirectional:... WIP" };

namespace vl {
ProgressivePathtrace::ProgressivePathtrace()
{
	for (size_t i = 0; i < c_framesInFlight; i++) {
		descSet[i] = Layouts->doubleStorageImage.AllocDescriptorSet();
		DEBUG_NAME(descSet[i], "area lights storage images");
	}
}

void ProgressivePathtrace::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, int32 frame)
{
	result[sceneDesc.frameIndex].TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eFragmentShader,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	auto extent = result[sceneDesc.frameIndex].extent;

	switch (const_pathtraceMode) {
		case PtMode::Naive:
			StaticPipes::Get<NaivePathtracePipe>().Draw(cmdBuffer, sceneDesc, descSet[sceneDesc.frameIndex], extent,
				frame, std::max(*cons_pathtraceBounces, 0));
			break;
		case PtMode::Stochastic:
			StaticPipes::Get<StochasticPathtracePipe>().Draw(cmdBuffer, sceneDesc, descSet[sceneDesc.frameIndex],
				extent, frame, std::max(*cons_pathtraceBounces, 0));
			break;
		case PtMode::Bdpt:
			StaticPipes::Get<BdptPipe>().Draw(cmdBuffer, sceneDesc, descSet[sceneDesc.frameIndex], extent, frame,
				std::max(*cons_pathtraceBounces, 0));
			break;
	}

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
