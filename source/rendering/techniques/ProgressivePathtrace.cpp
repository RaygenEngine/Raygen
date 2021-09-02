#include "ProgressivePathtrace.h"

#include "rendering/Layouts.h"
#include "rendering/pipes/AccumulationPipe.h"
#include "rendering/pipes/NaivePathtracePipe.h"
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


ProgressivePathtrace::ProgressivePathtrace()
{
	pathtracedDescSet = DescriptorLayouts->_1storageImage.AllocDescriptorSet();
	DEBUG_NAME_AUTO(pathtracedDescSet);

	inputOutputDescSet = DescriptorLayouts->_2storageImage.AllocDescriptorSet();
	DEBUG_NAME_AUTO(inputOutputDescSet);

	viewerDescSet = DescriptorLayouts->_1uniformBuffer.AllocDescriptorSet();
	DEBUG_NAME_AUTO(viewerDescSet);

	auto uboSize = sizeof(UBO_viewer);

	viewer = vl::RBuffer{ uboSize, vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	rvk::writeDescriptorBuffer(viewerDescSet, 0u, viewer.handle());
}

void ProgressivePathtrace::RecordCmd(
	vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, int32 samples, int32 bounces, PtMode mode)
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	if (updateViewer.Access()) {
		UBO_viewer data = {
			sceneDesc.viewer.ubo.viewInv,
			sceneDesc.viewer.ubo.projInv,
			0,
		};

		viewer.UploadData(&data, sizeof(UBO_viewer));
		iteration = 0;
	}

	pathtraced.TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	progressive.TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	auto extent = progressive.extent;

	// WIP: if there is no geometry this is validation error here

	switch (mode) {
		case PtMode::Naive:
			StaticPipes::Get<NaivePathtracePipe>().RecordCmd(cmdBuffer, extent, sceneDesc, pathtracedDescSet,
				viewerDescSet, iteration, std::max(samples, 0), std::max(bounces, 0));
			break;
		case PtMode::Stochastic:
			StaticPipes::Get<StochasticPathtracePipe>().RecordCmd(cmdBuffer, sceneDesc, extent, pathtracedDescSet,
				viewerDescSet, iteration, std::max(samples, 0), std::max(bounces, 0));
			break;
	}

	StaticPipes::Get<AccumulationPipe>().RecordCmd(cmdBuffer, extent, inputOutputDescSet, iteration);

	pathtraced.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eFragmentShader);

	progressive.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eFragmentShader);

	iteration += 1;
}

void ProgressivePathtrace::Resize(vk::Extent2D extent)
{
	pathtraced = RImage2D("Pathtraced Result",
		vk::Extent2D{ static_cast<uint32>(extent.width), static_cast<uint32>(extent.height) },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

	progressive = RImage2D("Pathtraced Progressive",
		vk::Extent2D{ static_cast<uint32>(extent.width), static_cast<uint32>(extent.height) },
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
