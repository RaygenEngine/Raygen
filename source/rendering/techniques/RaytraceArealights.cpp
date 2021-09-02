#include "RaytraceArealights.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Layouts.h"
#include "rendering/pipes/ArealightsPipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/pipes/SvgfAtrousPipe.h"
#include "rendering/pipes/SvgfMomentsPipe.h"
#include "rendering/pipes/SvgfModulatePipe.h"

namespace vl {
RaytraceArealights::RaytraceArealights()
{
	pathtracingInputDescSet = DescriptorLayouts->_1storageImage.AllocDescriptorSet();
	DEBUG_NAME_AUTO(pathtracingInputDescSet);
}

void RaytraceArealights::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, int32 samples,
	float minColorAlpha, float minMomentsAlpha, int32 totalIterations, float phiColor, float phiNormal)
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	auto extent = pathtracedResult.extent;

	pathtracedResult.TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	static int32 seed = 0;

	StaticPipes::Get<ArealightsPipe>().RecordCmd(
		cmdBuffer, sceneDesc, extent, pathtracingInputDescSet, samples, seed++);

	pathtracedResult.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eFragmentShader);

	svgFiltering.RecordCmd(cmdBuffer, sceneDesc, minColorAlpha, minMomentsAlpha, totalIterations, phiColor, phiNormal);
}

void RaytraceArealights::Resize(vk::Extent2D extent)
{
	pathtracedResult = RImage2D("Pathtraced (per iteration)", vk::Extent2D{ extent.width, extent.height },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

	svgFiltering.AttachInputImage(pathtracedResult);

	rvk::writeDescriptorImages(pathtracingInputDescSet, 0u, { pathtracedResult.view() },
		vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);
}

} // namespace vl
