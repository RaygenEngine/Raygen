#include "RaytraceArealights.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Layouts.h"
#include "rendering/pipes/ArealightsPipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/pipes/SvgfAtrousPipe.h"

// TODO: use specific for each technique instance and waitIdle() resize
ConsoleVariable<float> cons_arealightsScale{ "r.arealights.scale", 1.f, "Set arealights scale" };

namespace vl {
RaytraceArealights::RaytraceArealights()
{
	imagesDescSet = DescriptorLayouts->_3storageImage.AllocDescriptorSet();
	DEBUG_NAME(imagesDescSet, "ProgArealights storage descriptor set");
}

void RaytraceArealights::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	COMMAND_SCOPE_AUTO(cmdBuffer);
	// CHECK: this should not run if there are no area lights in the scene

	StaticPipes::Get<ArealightsPipe>().RecordCmd(cmdBuffer, sceneDesc, progressive.extent, imagesDescSet, iteration);

	static ConsoleVariable<int32> cons_iters{ "r.arealights.svgf.iterations", 4,
		"Controls how many times to apply svgf atrous filter." };

	auto times = *cons_iters;
	for (int32 i = 0; i < times; ++i) {
		svgfRenderPassInstance[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
			//
			StaticPipes::Get<SvgfAtrousPipe>().RecordCmd(cmdBuffer, sceneDesc, descriptorSets[i % 2], i, times);
		});
	}

	iteration += 1;
}

void RaytraceArealights::Resize(vk::Extent2D extent)
{
	progressive = RImage2D("ArealightProg",
		vk::Extent2D{ static_cast<uint32>(extent.width * cons_arealightsScale),
			static_cast<uint32>(extent.height * cons_arealightsScale) },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eGeneral);


	momentsBuffer = RImage2D("Moments Buffer", extent, vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eGeneral);

	for (size_t i = 0; i < c_framesInFlight; ++i) {
		svgfRenderPassInstance[i] = PassLayouts->svgf.CreatePassInstance(extent.width, extent.height);
	}

	swappingImages[0] = RImage2D("SVGF 0", extent, vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eGeneral);

	swappingImages[1] = RImage2D("SVGF 1", extent, vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eGeneral);

	for (size_t j = 0; j < 2; ++j) {
		descriptorSets[j] = DescriptorLayouts->_4storageImage.AllocDescriptorSet();

		rvk::writeDescriptorImages(descriptorSets[j], 0u,
			{
				progressive.view(),
				momentsBuffer.view(),
				swappingImages[(j + 0) % 2].view(),
				swappingImages[(j + 1) % 2].view(),
			},
			nullptr, vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);
	}

	rvk::writeDescriptorImages(imagesDescSet, 0u,
		{
			swappingImages[0].view(),
			progressive.view(),
			momentsBuffer.view(),
		},
		nullptr, vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);

	iteration = 0;
}

} // namespace vl
