#include "TestSVGFProgPT.h"

#include "rendering/Layouts.h"
#include "rendering/pipes/MomentsBufferCalculationPipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/pipes/StochasticPathtracePipe.h"
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

	pathtraced.TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	momentsHistory.TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	progressiveVariance.TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eFragmentShader,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	auto extent = progressiveVariance.extent;

	StaticPipes::Get<StochasticPathtracePipe>().RecordCmd(cmdBuffer, sceneDesc, extent, pathtracedDescSet,
		viewerDescSet, iteration, std::max(samples, 0), std::max(bounces, 0));

	pathtraced.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eFragmentShader);

	StaticPipes::Get<MomentsBufferCalculationPipe>().RecordCmd(cmdBuffer, extent, inputOutputsDescSet, sceneDesc);

	momentsHistory.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eFragmentShader);

	progressiveVariance.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
		vk::PipelineStageFlagBits::eFragmentShader);

	iteration += 1;
}

void TestSVGFProgPT::Resize(vk::Extent2D extent)
{
	pathtraced = RImage2D("Pathtraced Result",
		vk::Extent2D{ static_cast<uint32>(extent.width), static_cast<uint32>(extent.height) },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

	progressiveVariance = RImage2D("progressiveVariance",
		vk::Extent2D{ static_cast<uint32>(extent.width), static_cast<uint32>(extent.height) },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

	momentsHistory = RImage2D("momentsHistory",
		vk::Extent2D{ static_cast<uint32>(extent.width), static_cast<uint32>(extent.height) },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

	rvk::writeDescriptorImages(
		pathtracedDescSet, 0u, { pathtraced.view() }, vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);

	rvk::writeDescriptorImages(inputOutputsDescSet, 0u, { pathtraced.view() });

	rvk::writeDescriptorImages(inputOutputsDescSet, 1u,
		{
			progressiveVariance.view(),
			momentsHistory.view(),
		},
		vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);

	iteration = 0;
}

} // namespace vl
