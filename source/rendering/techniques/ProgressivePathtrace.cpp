#include "ProgressivePathtrace.h"

#include "rendering/pipes/BdptPipe.h"
#include "rendering/pipes/NaivePathtracePipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/pipes/StochasticPathtracePipe.h"
#include "rendering/scene/SceneIrragrid.h"

namespace {
enum class PtMode
{
	Naive,
	Stochastic,
	Bdpt,
};

struct UBO_viewer {
	glm::mat4 viewInv;
	glm::mat4 projInv;
	float offset;
};
}; // namespace

ConsoleVariable<float> cons_pathtraceScale{ "r.pathtracing.scale", 1.f, "Set scale of the pathtraced image." };
ConsoleVariable<int32> cons_pathtraceBounces{ "r.pathtracing.bounces", 1,
	"Set the number of bounces of the pathtracer." };
ConsoleVariable<int32> cons_pathtraceSamples{ "r.pathtracing.samples", 1,
	"Set the number of samples of the pathtracer." };
ConsoleVariable<PtMode> const_pathtraceMode{ "r.pathtracing.mode", PtMode::Stochastic,
	"Set the pathtracing mode. Naive: unidirectional, not entirely naive, it just lacks direct light sampling,"
	"Stochastic: similar to naive but uses direct light and light MIS,"
	"Bpdt: Bidirectional:... Work in progress" };

namespace vl {


ProgressivePathtrace::ProgressivePathtrace()
{
	progressiveDescSet = Layouts->singleStorageImage.AllocDescriptorSet();
	DEBUG_NAME(progressiveDescSet, "progressive pathtrace storage image desc set");

	viewerDescSet = Layouts->singleUboDescLayout.AllocDescriptorSet();
	DEBUG_NAME(progressiveDescSet, "progressive pathtrace camera desc set");

	auto uboSize = sizeof(UBO_viewer);

	viewer = vl::RBuffer{ uboSize, vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	rvk::writeDescriptorBuffer(viewerDescSet, 0u, viewer.handle());
}

void ProgressivePathtrace::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, int32 iteration)
{
	progressive.TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	auto extent = progressive.extent;

	// CHECK: if there is no geometry this is validation error here

	switch (const_pathtraceMode) {
		case PtMode::Naive:
			StaticPipes::Get<NaivePathtracePipe>().Draw(cmdBuffer, sceneDesc, progressiveDescSet, viewerDescSet, extent,
				iteration, std::max(*cons_pathtraceSamples, 0), std::max(*cons_pathtraceBounces, 0));
			break;
		case PtMode::Stochastic:
			StaticPipes::Get<StochasticPathtracePipe>().Draw(cmdBuffer, sceneDesc, progressiveDescSet, viewerDescSet,
				extent, iteration, std::max(*cons_pathtraceSamples, 0), std::max(*cons_pathtraceBounces, 0));
			break;
		case PtMode::Bdpt:
			StaticPipes::Get<BdptPipe>().Draw(
				cmdBuffer, sceneDesc, progressiveDescSet, extent, iteration, std::max(*cons_pathtraceBounces, 0));
			break;
	}

	progressive.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eFragmentShader);
}

void ProgressivePathtrace::Resize(vk::Extent2D extent)
{
	progressive = RImage2D("PathtraceProg",
		vk::Extent2D{ static_cast<uint32>(extent.width * cons_pathtraceScale),
			static_cast<uint32>(extent.height * cons_pathtraceScale) },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

	rvk::writeDescriptorImages(
		progressiveDescSet, 0u, { progressive.view() }, vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);
}

void ProgressivePathtrace::UpdateViewer(const glm::mat4& viewInv, const glm::mat4& projInv, float offset)
{
	UBO_viewer data = {
		viewInv,
		projInv,
		offset,
	};

	viewer.UploadData(&data, sizeof(UBO_viewer));
}
} // namespace vl
