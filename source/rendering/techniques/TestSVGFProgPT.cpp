#include "TestSVGFProgPT.h"

#include "rendering/Layouts.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/pipes/StochasticPathtracePipe.h"
#include "rendering/pipes/SvgfAtrousPipe.h"
#include "rendering/pipes/SvgfMomentsPipe.h"
#include "rendering/pipes/SvgfModulatePipe.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneCamera.h"

namespace {
struct UBO_viewer {
	glm::mat4 viewInv;
	glm::mat4 projInv;
	float offset;
};
}; // namespace

namespace vl {
TestSVGFProgPT::TestSVGFProgPT()
{
	pathtracingInputDescSet = DescriptorLayouts->_1storageImage.AllocDescriptorSet();
	DEBUG_NAME_AUTO(pathtracingInputDescSet);

	inputOutputsDescSet = DescriptorLayouts->_1imageSampler_3storageImage.AllocDescriptorSet();
	DEBUG_NAME_AUTO(inputOutputsDescSet);

	viewerDescSet = DescriptorLayouts->_1uniformBuffer.AllocDescriptorSet();
	DEBUG_NAME_AUTO(viewerDescSet);

	for (size_t j = 0; j < 2; ++j) {
		descriptorSets[j] = DescriptorLayouts->_3storageImage.AllocDescriptorSet();
		DEBUG_NAME(descriptorSets[j], "SvgfAtrousDescSet" + j);
	}

	auto uboSize = sizeof(UBO_viewer);

	viewer = vl::RBuffer{ uboSize, vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	rvk::writeDescriptorBuffer(viewerDescSet, 0u, viewer.handle());
}


void TestSVGFProgPT::RecordCmd(
	vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, int32 samples, int32 bounces)
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	static ConsoleVariable<int32> cons_iters{ "r.rtxRenderer.svgf.iterations", 4,
		"Controls how many times to apply svgf atrous filter." };

	if (updateViewer.Access()) {
		UBO_viewer data = {
			sceneDesc.viewer.ubo.viewInv,
			sceneDesc.viewer.ubo.projInv,
			0,
		};

		viewer.UploadData(&data, sizeof(UBO_viewer));
	}

	auto extent = pathtracedResult.extent;


	pathtracedResult.TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	StaticPipes::Get<StochasticPathtracePipe>().RecordCmd(cmdBuffer, sceneDesc, extent, pathtracingInputDescSet,
		viewerDescSet, iteration, std::max(samples, 0), std::max(bounces, 0));

	pathtracedResult.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eFragmentShader);

	progressive.TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	momentsHistory.TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	StaticPipes::Get<SvgfMomentsPipe>().RecordCmd(cmdBuffer, extent, sceneDesc, inputOutputsDescSet, iteration == 0);

	// Atrous filter
	auto times = std::max(*cons_iters, 1);

	svgfRenderPassInstance[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
		for (int32 i = 0; i < times; ++i) {
			StaticPipes::Get<SvgfAtrousPipe>().RecordCmd(cmdBuffer, sceneDesc, descriptorSets[i % 2], i, times, true);
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

void TestSVGFProgPT::Resize(vk::Extent2D extent)
{
	pathtracedResult = RImage2D("Pathtraced (per iteration)", vk::Extent2D{ extent.width, extent.height },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

	progressive = RImage2D("ProgressiveVariance", vk::Extent2D{ extent.width, extent.height },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

	momentsHistory = RImage2D("MomentsHistory", vk::Extent2D{ extent.width, extent.height },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

	for (size_t i = 0; i < c_framesInFlight; ++i) {
		svgfRenderPassInstance[i] = PassLayouts->svgf.CreatePassInstance(extent.width, extent.height);
	}

	swappingImages[0] = RImage2D("Svgf0", extent, vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eGeneral);
	swappingImages[1] = RImage2D("Svgf1", extent, vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eGeneral);

	rvk::writeDescriptorImages(pathtracingInputDescSet, 0u, { pathtracedResult.view() },
		vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);

	rvk::writeDescriptorImages(inputOutputsDescSet, 0u, { pathtracedResult.view() },
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
