#include "DirlightBlend.h"

#include "rendering/StaticPipes.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/core/PipeUtl.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/SceneDirlight.h"

namespace vl {
vk::UniquePipelineLayout DirlightBlend::MakePipelineLayout()
{
	return rvk::makeLayoutNoPC({
		Layouts->renderAttachmentsLayout.handle(),
		Layouts->singleUboDescLayout.handle(),
		Layouts->singleUboDescLayout.handle(),
		Layouts->singleSamplerDescLayout.handle(),
	});
}

vk::UniquePipeline DirlightBlend::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/light/dirlight.shader");
	gpuShader.onCompile = [&]() {
		StaticPipes::Recompile<DirlightBlend>();
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

	return rvk::makePostProcPipeline(gpuShader.shaderStages, StaticPipes::GetLayout<DirlightBlend>(),
		*Layouts->directLightPassLayout.compatibleRenderPass, colorBlending);
}

void DirlightBlend::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const
{
	auto camDescSet = sceneDesc.viewer.uboDescSet[sceneDesc.frameIndex];

	auto& pipeLayout = StaticPipes::GetLayout<DirlightBlend>();

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline());

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, pipeLayout, 0u, 1u, &sceneDesc.attachmentsDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeLayout, 1u, 1u, &camDescSet, 0u, nullptr);

	for (auto dl : sceneDesc->Get<SceneDirlight>()) {
		cmdBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics, pipeLayout, 2u, 1u, &dl->uboDescSet[sceneDesc.frameIndex], 0u, nullptr);

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeLayout, 3u, 1u,
			&dl->shadowmapDescSet[sceneDesc.frameIndex], 0u, nullptr);

		// big triangle
		cmdBuffer.draw(3u, 1u, 0u, 0u);
	}
}


} // namespace vl
