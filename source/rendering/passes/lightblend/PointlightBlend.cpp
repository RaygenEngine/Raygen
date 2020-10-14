#include "PointlightBlend.h"

#include "rendering/core/PipeUtl.h"
#include "rendering/Layouts.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/StaticPipes.h"
#include "rendering/scene/ScenePointlight.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/Scene.h"


vk::UniquePipelineLayout vl::PointlightBlend::MakePipelineLayout()
{
	return rvk::makeLayoutNoPC({
		Layouts->renderAttachmentsLayout.handle(),
		Layouts->singleUboDescLayout.handle(),
		Layouts->singleUboDescLayout.handle(),
		Layouts->accelLayout.handle(),
	});
}

vk::UniquePipeline vl::PointlightBlend::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/light/pointlight.shader");
	gpuShader.onCompile = [&]() {
		StaticPipes::Recompile<PointlightBlend>();
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

	return rvk::makePostProcPipeline(gpuShader.shaderStages, StaticPipes::GetLayout<PointlightBlend>(),
		*Layouts->rasterDirectPassLayout.compatibleRenderPass, colorBlending);
}

void vl::PointlightBlend::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	auto camDescSet = sceneDesc.viewer->descSet[sceneDesc.frameIndex];

	auto& pipeLayout = StaticPipes::GetLayout<PointlightBlend>();

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, StaticPipes::Get<PointlightBlend>());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeLayout, 0u, 1u, &sceneDesc.attDesc, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeLayout, 1u, 1u, &camDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, pipeLayout, 3u, 1u, &sceneDesc.scene->sceneAsDescSet, 0u, nullptr);

	for (auto pl : sceneDesc->pointlights.elements) {
		if (!pl) {
			continue;
		}

		cmdBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics, pipeLayout, 2u, 1u, &pl->descSet[sceneDesc.frameIndex], 0u, nullptr);

		// big triangle
		cmdBuffer.draw(3u, 1u, 0u, 0u);
	}
}
