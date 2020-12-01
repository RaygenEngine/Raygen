#include "SpotlightPipe.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/core/PipeUtl.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/SceneSpotlight.h"

namespace vl {

vk::UniquePipelineLayout SpotlightPipe::MakePipelineLayout()
{
	return rvk::makeLayoutNoPC({
		Layouts->mainPassLayout.internalDescLayout.handle(),
		Layouts->singleUboDescLayout.handle(),
		Layouts->singleUboDescLayout.handle(),
		Layouts->singleSamplerDescLayout.handle(),
	});
}

vk::UniquePipeline SpotlightPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/light/spotlight.shader");
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

	return rvk::makePostProcPipeline(gpuShader.shaderStages, StaticPipes::GetLayout<SpotlightPipe>(),
		*Layouts->mainPassLayout.compatibleRenderPass, colorBlending, 1u);
}

void SpotlightPipe::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const
{
	auto camDescSet = sceneDesc.viewer.uboDescSet[sceneDesc.frameIndex];

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout(), 1u, 1u, &camDescSet, 0u, nullptr);

	for (auto sp : sceneDesc->Get<SceneSpotlight>()) {
		cmdBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics, layout(), 2u, 1u, &sp->uboDescSet[sceneDesc.frameIndex], 0u, nullptr);

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout(), 3u, 1u,
			&sp->shadowmapDescSet[sceneDesc.frameIndex], 0u, nullptr);

		// big triangle
		cmdBuffer.draw(3u, 1u, 0u, 0u);
	}
}

} // namespace vl
