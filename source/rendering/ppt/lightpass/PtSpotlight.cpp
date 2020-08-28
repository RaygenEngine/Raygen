#include "pch.h"
#include "PtSpotlight.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Device.h"
#include "rendering/Renderer.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneSpotlight.h"
#include "rendering/structures/Depthmap.h"
#include "rendering/scene/SceneCamera.h"

namespace vl {
void PtSpotlight::MakeLayout()
{
	std::array layouts{
		Layouts->gbufferDescLayout.handle(),
		Layouts->singleUboDescLayout.handle(),
		Layouts->singleUboDescLayout.handle(),
		Layouts->singleSamplerDescLayout.handle(),
		Layouts->accelLayout.handle(),
	};

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.setSetLayouts(layouts);

	m_pipelineLayout = Device->createPipelineLayoutUnique(pipelineLayoutInfo);
}

void PtSpotlight::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/ppt/light/spotlight.shader");
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
		.setAttachments(colorBlendAttachment)
		.setBlendConstants({ 0.f, 0.f, 0.f, 0.f });

	Utl_CreatePipeline(gpuShader, colorBlending);
}

void PtSpotlight::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, vk::DescriptorSet gbufferDescSet)
{
	auto camera = sceneDesc.viewer;

	if (!camera) {
		return;
	}

	// WIP:
	auto descSet = camera->descSet[sceneDesc.frameIndex];

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, 1u, &gbufferDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 1u, 1u, &descSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 4u, 1u,
		&sceneDesc.scene->sceneAsDescSet, 0u, nullptr);

	for (auto sl : sceneDesc->spotlights.elements) {
		if (!sl) {
			continue;
		}

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 2u, 1u,
			&sl->descSet[sceneDesc.frameIndex], 0u, nullptr);

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 3u, 1u,
			&sl->shadowmap[sceneDesc.frameIndex].descSet, 0u, nullptr);


		// draw call (triangle)
		cmdBuffer.draw(3u, 1u, 0u, 0u);
	}
}
} // namespace vl
