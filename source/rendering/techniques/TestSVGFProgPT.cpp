#include "TestSVGFProgPT.h"

#include "rendering/Layouts.h"
#include "rendering/pipes/MomentsBufferCalculationPipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/pipes/StochasticPathtracePipe.h"
#include "rendering/pipes/SvgfAtrousPipe.h"
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
	pathtracedDescSet = DescriptorLayouts->_1storageImage.AllocDescriptorSet();
	DEBUG_NAME_AUTO(pathtracedDescSet);

	inputOutputsDescSet = DescriptorLayouts->_1imageSampler_2storageImage.AllocDescriptorSet();
	DEBUG_NAME_AUTO(inputOutputsDescSet);

	viewerDescSet = DescriptorLayouts->_1uniformBuffer.AllocDescriptorSet();
	DEBUG_NAME_AUTO(viewerDescSet);

	auto uboSize = sizeof(UBO_viewer);

	viewer = vl::RBuffer{ uboSize, vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	rvk::writeDescriptorBuffer(viewerDescSet, 0u, viewer.handle());
}


void TestSVGFProgPT::RecordCmd(
	vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, int32 samples, int32 bounces)
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	if (updateViewer.Access()) {
		UBO_viewer data = {
			sceneDesc.viewer.ubo.viewInv,
			sceneDesc.viewer.ubo.projInv,
			0,
		};

		viewer.UploadData(&data, sizeof(UBO_viewer));
	}

	auto extent = progressiveVariance.extent;

	progressiveVariance.TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eFragmentShader,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	momentsHistory.TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	StaticPipes::Get<StochasticPathtracePipe>().RecordCmd(cmdBuffer, sceneDesc, extent, pathtracedDescSet,
		viewerDescSet, iteration, std::max(samples, 0), std::max(bounces, 0));

	swappingImages[0].TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal);

	StaticPipes::Get<MomentsBufferCalculationPipe>().RecordCmd(cmdBuffer, extent, inputOutputsDescSet, sceneDesc);

	swappingImages[0].TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral);

	static ConsoleVariable<int32> cons_iters{ "r.rtxRenderer.svgf.iterations", 4,
		"Controls how many times to apply svgf atrous filter." };

	// Atrous filter
	auto times = std::max(*cons_iters, 1);
	for (int32 i = 0; i < times; ++i) {
		svgfRenderPassInstance[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
			StaticPipes::Get<SvgfAtrousPipe>().RecordCmd(cmdBuffer, sceneDesc, descriptorSets[i % 2], i, times);
		});
	}

	progressiveVariance.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
		vk::PipelineStageFlagBits::eFragmentShader);

	momentsHistory.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eFragmentShader);


	iteration += 1;
}

void TestSVGFProgPT::Resize(vk::Extent2D extent)
{
	progressiveVariance = RImage2D("progressiveVariance",
		vk::Extent2D{ static_cast<uint32>(extent.width), static_cast<uint32>(extent.height) },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

	momentsHistory = RImage2D("momentsHistory",
		vk::Extent2D{ static_cast<uint32>(extent.width), static_cast<uint32>(extent.height) },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

	for (size_t i = 0; i < c_framesInFlight; ++i) {
		svgfRenderPassInstance[i] = PassLayouts->svgf.CreatePassInstance(extent.width, extent.height);
	}

	swappingImages[0] = RImage2D("Svgf0", extent, vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eGeneral);
	swappingImages[1] = RImage2D("Svgf1", extent, vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eGeneral);

	rvk::writeDescriptorImages(pathtracedDescSet, 0u, { swappingImages[0].view() }, vk::DescriptorType::eStorageImage,
		vk::ImageLayout::eGeneral);

	rvk::writeDescriptorImages(inputOutputsDescSet, 0u, { swappingImages[0].view() },
		vk::DescriptorType::eCombinedImageSampler, vk::ImageLayout::eShaderReadOnlyOptimal);

	rvk::writeDescriptorImages(inputOutputsDescSet, 1u,
		{
			progressiveVariance.view(),
			momentsHistory.view(),
		},
		vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);


	for (size_t j = 0; j < 2; ++j) {
		descriptorSets[j] = DescriptorLayouts->_4storageImage.AllocDescriptorSet();

		rvk::writeDescriptorImages(descriptorSets[j], 0u,
			{
				progressiveVariance.view(),
				momentsHistory.view(),
				swappingImages[(j + 0) % 2].view(),
				swappingImages[(j + 1) % 2].view(),
			},
			nullptr, vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);
	}

	iteration = 0;
}

} // namespace vl
