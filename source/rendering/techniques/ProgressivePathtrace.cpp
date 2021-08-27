#include "ProgressivePathtrace.h"

#include "rendering/Layouts.h"
#include "rendering/pipes/AccumulationPipe.h"
#include "rendering/pipes/BdptPipe.h"
#include "rendering/pipes/NaivePathtracePipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/pipes/StochasticPathtracePipe.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneCamera.h"

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
	pathtracedDescSet = Layouts->singleStorageImage.AllocDescriptorSet();
	DEBUG_NAME(pathtracedDescSet, "pathtrace storage image desc set");

	inputOutputDescSet = Layouts->doubleStorageImage.AllocDescriptorSet();
	DEBUG_NAME(inputOutputDescSet, "progressive pathtrace storage image desc set");

	viewerDescSet = Layouts->singleUboDescLayout.AllocDescriptorSet();
	DEBUG_NAME(viewerDescSet, "pathtrace viewer desc set");

	auto uboSize = sizeof(UBO_viewer);

	viewer = vl::RBuffer{ uboSize, vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	rvk::writeDescriptorBuffer(viewerDescSet, 0u, viewer.handle());
}

void ProgressivePathtrace::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	if (updateViewer.Access()) {
		UBO_viewer data = {
			sceneDesc.viewer.ubo.viewInv,
			sceneDesc.viewer.ubo.projInv,
			0,
		};

		viewer.UploadData(&data, sizeof(UBO_viewer));
	}

	pathtraced.TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	progressive.TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	auto extent = progressive.extent;

	// CHECK: if there is no geometry this is validation error here

	CMDSCOPE_BEGIN(cmdBuffer, "Pathtracer commands");

	switch (const_pathtraceMode) {
		case PtMode::Naive:
			StaticPipes::Get<NaivePathtracePipe>().Draw(cmdBuffer, sceneDesc, pathtracedDescSet, viewerDescSet, extent,
				iteration, std::max(*cons_pathtraceSamples, 0), std::max(*cons_pathtraceBounces, 0));
			break;
		case PtMode::Stochastic:
			StaticPipes::Get<StochasticPathtracePipe>().Draw(cmdBuffer, sceneDesc, pathtracedDescSet, viewerDescSet,
				extent, iteration, std::max(*cons_pathtraceSamples, 0), std::max(*cons_pathtraceBounces, 0));
			break;
		case PtMode::Bdpt:
			StaticPipes::Get<BdptPipe>().Draw(
				cmdBuffer, sceneDesc, pathtracedDescSet, extent, iteration, std::max(*cons_pathtraceBounces, 0));
			break;
	}

	CMDSCOPE_END(cmdBuffer);

	CMDSCOPE_BEGIN(cmdBuffer, "Compute Accumulation");

	StaticPipes::Get<AccumulationPipe>().Draw(cmdBuffer, inputOutputDescSet, extent, iteration);

	CMDSCOPE_END(cmdBuffer);

	pathtraced.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eFragmentShader);

	progressive.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eFragmentShader);

	iteration += 1;
}

void ProgressivePathtrace::Resize(vk::Extent2D extent)
{
	pathtraced = RImage2D("Pathtraced Result",
		vk::Extent2D{ static_cast<uint32>(extent.width * cons_pathtraceScale),
			static_cast<uint32>(extent.height * cons_pathtraceScale) },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

	progressive = RImage2D("Pathtraced Progressive",
		vk::Extent2D{ static_cast<uint32>(extent.width * cons_pathtraceScale),
			static_cast<uint32>(extent.height * cons_pathtraceScale) },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

	rvk::writeDescriptorImages(
		pathtracedDescSet, 0u, { pathtraced.view() }, vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);

	rvk::writeDescriptorImages(inputOutputDescSet, 0u,
		{
			pathtraced.view(),
			progressive.view(),
		},
		vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);

	iteration = 0;
}
} // namespace vl
