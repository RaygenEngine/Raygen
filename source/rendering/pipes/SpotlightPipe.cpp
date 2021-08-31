#include "SpotlightPipe.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneSpotlight.h"

namespace vl {

vk::UniquePipelineLayout SpotlightPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayoutNoPC({
		DescriptorLayouts->global.handle(),
		PassLayouts->main.internalDescLayout.handle(),
		DescriptorLayouts->_1uniformBuffer.handle(),
		DescriptorLayouts->_1imageSampler.handle(),
	});
}

vk::UniquePipeline SpotlightPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/lighting/spotlight.shader");
	gpuShader.onCompile = [&]() {
		StaticPipes::Recompile<SpotlightPipe>();
	};

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment
		.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
						   | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) //
		.setBlendEnable(VK_TRUE)
		.setSrcColorBlendFactor(vk::BlendFactor::eOne)
		.setDstColorBlendFactor(vk::BlendFactor::eOne)
		.setColorBlendOp(vk::BlendOp::eAdd)
		.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
		.setDstAlphaBlendFactor(vk::BlendFactor::eOne)
		.setAlphaBlendOp(vk::BlendOp::eAdd);

	vk::PipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending
		.setLogicOpEnable(VK_FALSE) //
		.setLogicOp(vk::LogicOp::eCopy)
		.setAttachmentCount(1u)
		.setPAttachments(&colorBlendAttachment)
		.setBlendConstants({ 0.f, 0.f, 0.f, 0.f });

	return rvk::makeGraphicsPipeline(gpuShader.shaderStages, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, &colorBlending, nullptr, layout(), PassLayouts->main.compatibleRenderPass.get(), 1u);
}

void SpotlightPipe::RecordCmd(
	vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, vk::DescriptorSet inputDescSet) const
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout(), 0u,
		{
			sceneDesc.globalDesc,
			inputDescSet,
		},
		nullptr);

	for (auto sp : sceneDesc->Get<SceneSpotlight>()) {
		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout(), 2u,
			{
				sp->uboDescSet[sceneDesc.frameIndex],
				sp->shadowmapDescSet[sceneDesc.frameIndex], // CHECK: if this is sp static, use 0 index?
			},
			nullptr);

		// big triangle
		cmdBuffer.draw(3u, 1u, 0u, 0u);
	}
}

} // namespace vl
