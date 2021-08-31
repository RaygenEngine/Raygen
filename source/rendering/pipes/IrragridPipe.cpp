#include "IrragridPipe.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneIrragrid.h"

namespace vl {

vk::UniquePipelineLayout IrragridPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayoutNoPC({
		DescriptorLayouts->global.handle(),
		PassLayouts->main.internalDescLayout.handle(),
		DescriptorLayouts->_1uniformBuffer.handle(),
		DescriptorLayouts->_1imageSampler.handle(),
	});
}

vk::UniquePipeline IrragridPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/lighting/irragrid.shader");
	gpuShader.onCompile = [&]() {
		StaticPipes::Recompile<IrragridPipe>();
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
		.setAttachments(colorBlendAttachment)
		.setBlendConstants({ 0.f, 0.f, 0.f, 0.f });


	return rvk::makeGraphicsPipeline(gpuShader.shaderStages, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, &colorBlending, nullptr, layout(), PassLayouts->main.compatibleRenderPass.get(), 2u);
}

void IrragridPipe::RecordCmd(
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

	for (auto ig : sceneDesc->Get<SceneIrragrid>()) {

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout(), 2u,
			{
				ig->uboDescSet[sceneDesc.frameIndex],
				ig->irradianceSamplerDescSet,
			},
			nullptr);

		// big triangle
		cmdBuffer.draw(3u, 1u, 0u, 0u);
	}
}

} // namespace vl
