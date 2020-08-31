#include "pch.h"
#include "DirlightBlend.h"

#include "rendering/core/PipeUtl.h"
#include "rendering/Layouts.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/StaticPipes.h"
#include "rendering/scene/SceneDirectionalLight.h"
#include "rendering/scene/SceneCamera.h"

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
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/ppt/light/dirlight.shader");
	gpuShader.onCompile = [&]() {
		MakePipeline();
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

	return vk::UniquePipeline();
	// return rvk::makePostProcPipeline(
	//	gpuShader.shaderStages, colorBlending, StaticPipes::GetLayout<DirlightBlend>(), /**/);
}

void DirlightBlend::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	auto camera = sceneDesc.viewer;

	if (!camera) {
		return;
	}

	// WIP:
	auto descSet = camera->descSet[sceneDesc.frameIndex];

	auto& pipeLayout = StaticPipes::GetLayout<DirlightBlend>();

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, StaticPipes::Get<DirlightBlend>());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeLayout, 0u, 1u, &sceneDesc.attDesc, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeLayout, 1u, 1u, &descSet, 0u, nullptr);

	for (auto dl : sceneDesc->directionalLights.elements) {
		if (!dl) {
			continue;
		}

		cmdBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics, pipeLayout, 2u, 1u, &dl->descSet[sceneDesc.frameIndex], 0u, nullptr);

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeLayout, 3u, 1u,
			&dl->shadowmap[sceneDesc.frameIndex].descSet, 0u, nullptr);

		// draw call (triangle)
		cmdBuffer.draw(3u, 1u, 0u, 0u);
	}
}


} // namespace vl
