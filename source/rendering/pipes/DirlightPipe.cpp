#include "DirlightPipe.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneDirlight.h"

namespace vl {
vk::UniquePipelineLayout DirlightPipe::MakePipelineLayout()
{
	return rvk::makeLayoutNoPC({
		DescriptorLayouts->global.handle(),
		PassLayouts->main.internalDescLayout.handle(),
		DescriptorLayouts->_1uniformBuffer.handle(),
		DescriptorLayouts->_1imageSampler.handle(),
	});
}

vk::UniquePipeline DirlightPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/lighting/dirlight.shader");
	gpuShader.onCompile = [&]() {
		StaticPipes::Recompile<DirlightPipe>();
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

	return rvk::makePostProcPipeline(gpuShader.shaderStages, StaticPipes::GetLayout<DirlightPipe>(),
		*PassLayouts->main.compatibleRenderPass, colorBlending, 1u);
}

void DirlightPipe::RecordCmd(
	vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, vk::DescriptorSet inputDescSet) const
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	auto&& pipeLayout = StaticPipes::GetLayout<DirlightPipe>();

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeLayout, 0u,
		{
			sceneDesc.globalDesc,
			inputDescSet,
		},
		nullptr);

	for (auto dl : sceneDesc->Get<SceneDirlight>()) {
		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeLayout, 2u,
			{
				dl->uboDescSet[sceneDesc.frameIndex],
				dl->shadowmapDescSet[sceneDesc.frameIndex],
			},
			nullptr);

		// big triangle
		cmdBuffer.draw(3u, 1u, 0u, 0u);
	}
}

} // namespace vl
