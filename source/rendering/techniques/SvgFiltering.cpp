#include "SvgFiltering.h"

#include "rendering/Layouts.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/pipes/StochasticPathtracePipe.h"
#include "rendering/pipes/SvgfAtrousPipe.h"
#include "rendering/pipes/SvgfMomentsPipe.h"
#include "rendering/pipes/SvgfModulatePipe.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneCamera.h"


namespace vl {
SvgFiltering::SvgFiltering()
{
	inputOutputsDescSet = DescriptorLayouts->_1imageSampler_3storageImage.AllocDescriptorSet();
	DEBUG_NAME_AUTO(inputOutputsDescSet);

	for (size_t j = 0; j < 2; ++j) {
		descriptorSets[j] = DescriptorLayouts->_3storageImage.AllocDescriptorSet();
		DEBUG_NAME(descriptorSets[j], "SvgfAtrousDescSet" + j);
	}
}

void SvgFiltering::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, float minColorAlpha,
	float minMomentsAlpha, int32 totalIterations, float phiColor, float phiNormal, bool luminanceMode)
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	auto extent = progressive.extent;

	progressive.TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	momentsHistory.TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	StaticPipes::Get<SvgfMomentsPipe>().RecordCmd(
		cmdBuffer, extent, sceneDesc, inputOutputsDescSet, minColorAlpha, minMomentsAlpha, luminanceMode);

	svgfRenderPassInstance[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
		for (int32 i = 0; i < totalIterations; ++i) {
			StaticPipes::Get<SvgfAtrousPipe>().RecordCmd(
				cmdBuffer, sceneDesc, descriptorSets[i % 2], i, totalIterations, phiColor, phiNormal, luminanceMode);
		}

		cmdBuffer.nextSubpass(vk::SubpassContents::eInline);

		auto inputDescSet = svgfRenderPassInstance[sceneDesc.frameIndex].internalDescSet;

		StaticPipes::Get<SvgfModulatePipe>().RecordCmd(cmdBuffer, sceneDesc, inputDescSet);
	});


	progressive.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eFragmentShader);

	momentsHistory.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eFragmentShader);

	iteration += 1;
}

void SvgFiltering::AttachInputImage(const RImage2D& inputImage)
{
	auto extent = vk::Extent2D(inputImage.extent.width, inputImage.extent.height);

	progressive = RImage2D("ProgressiveVariance", vk::Extent2D{ extent.width, extent.height },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

	momentsHistory = RImage2D("MomentsHistory", vk::Extent2D{ extent.width, extent.height },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

	for (size_t i = 0; i < c_framesInFlight; ++i) {
		svgfRenderPassInstance[i] = PassLayouts->svgf.CreatePassInstance(extent.width, extent.height);
	}

	swappingImages[0] = RImage2D("Svgf0", extent, vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eGeneral);
	swappingImages[1] = RImage2D("Svgf1", extent, vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eGeneral);

	rvk::writeDescriptorImages(inputOutputsDescSet, 0u, { inputImage.view() },
		vk::DescriptorType::eCombinedImageSampler, vk::ImageLayout::eShaderReadOnlyOptimal);

	rvk::writeDescriptorImages(inputOutputsDescSet, 1u,
		{
			swappingImages[0].view(),
			progressive.view(),
			momentsHistory.view(),
		},
		vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);


	for (size_t j = 0; j < 2; ++j) {
		rvk::writeDescriptorImages(descriptorSets[j], 0u,
			{
				progressive.view(),
				swappingImages[(j + 0) % 2].view(),
				swappingImages[(j + 1) % 2].view(),
			},
			nullptr, vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);
	}

	iteration = 0;
}

} // namespace vl
