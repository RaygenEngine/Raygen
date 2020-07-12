#include "pch.h"
#include "PtSpotlight.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Renderer.h"
#include "rendering/scene/Scene.h"

namespace vl {
void PtSpotlight::MakeLayout()
{
	std::array layouts = { Layouts->gbufferDescLayout.setLayout.get(), Layouts->singleUboDescLayout.setLayout.get(),
		Layouts->singleUboDescLayout.setLayout.get(), Layouts->singleSamplerDescLayout.setLayout.get() };

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setSetLayoutCount(static_cast<uint32>(layouts.size())) //
		.setPSetLayouts(layouts.data())
		.setPushConstantRangeCount(0u)
		.setPPushConstantRanges(nullptr);

	m_pipelineLayout = Device->createPipelineLayoutUnique(pipelineLayoutInfo);
}

void PtSpotlight::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/ppt/light/spotlight.shader");
	gpuShader.onCompile = [&]() {
		MakePipeline();
	};

	// CHECK: be sure you use the correct blending operations (also check logic ops)
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

	Utl_CreatePipeline(gpuShader, colorBlending);
}

void PtSpotlight::Draw(vk::CommandBuffer cmdBuffer, uint32 frameIndex)
{
	if (!Scene->GetActiveCamera()) {
		return;
	}

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, 1u,
		&Renderer->GetGbuffer()->descSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 1u, 1u,
		&Scene->GetActiveCameraDescSet(), 0u, nullptr);

	for (auto sl : Scene->spotlights.elements) {
		if (!sl) {
			continue;
		}

		cmdBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 2u, 1u, &sl->descSets[frameIndex], 0u, nullptr);

		cmdBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 3u, 1u, &sl->shadowmap->descSet, 0u, nullptr);

		// draw call (triangle)
		cmdBuffer.draw(3u, 1u, 0u, 0u);
	}
}
} // namespace vl
