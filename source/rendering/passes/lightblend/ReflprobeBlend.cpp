#include "ReflprobeBlend.h"

#include "rendering/core/PipeUtl.h"
#include "rendering/Layouts.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/StaticPipes.h"
#include "rendering/scene/SceneReflProbe.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/scene/Scene.h"

vk::UniquePipelineLayout vl::ReflprobeBlend::MakePipelineLayout()
{
	return rvk::makeLayoutNoPC({
		Layouts->renderAttachmentsLayout.handle(),
		Layouts->singleUboDescLayout.handle(),
		Layouts->envmapLayout.handle(),
	});
}

vk::UniquePipeline vl::ReflprobeBlend::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/light/reflprobe.shader");
	gpuShader.onCompile = [&]() {
		StaticPipes::Recompile<ReflprobeBlend>();
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

	return rvk::makePostProcPipeline(gpuShader.shaderStages, StaticPipes::GetLayout<ReflprobeBlend>(),
		*Layouts->rasterDirectPassLayout.compatibleRenderPass, colorBlending);
}

void vl::ReflprobeBlend::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const
{
	auto camDescSet = sceneDesc.viewer->descSet[sceneDesc.frameIndex];

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout(), 0u, 1u, &sceneDesc.attDesc, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout(), 1u, 1u, &camDescSet, 0u, nullptr);

	for (auto rp : sceneDesc->reflProbs.elements) {
		if (rp) {
			cmdBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics, layout(), 2u, 1u, &rp->envmap.Lock().descriptorSet, 0u, nullptr);

			// big triangle
			cmdBuffer.draw(3u, 1u, 0u, 0u);
		}
	}
}
