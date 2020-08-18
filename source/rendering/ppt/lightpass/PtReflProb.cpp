#include "pch.h"
#include "PtReflProb.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Device.h"
#include "rendering/Renderer.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneReflectionProbe.h"
#include "rendering/scene/SceneCamera.h"

namespace vl {
void PtReflProb::MakeLayout()
{
	std::array layouts = { Layouts->gbufferDescLayout.setLayout.get(), Layouts->singleUboDescLayout.setLayout.get(),
		Layouts->envmapLayout.setLayout.get() };

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setSetLayoutCount(static_cast<uint32>(layouts.size())) //
		.setPSetLayouts(layouts.data())
		.setPushConstantRangeCount(0u)
		.setPPushConstantRanges(nullptr);

	m_pipelineLayout = Device->createPipelineLayoutUnique(pipelineLayoutInfo);
}

void PtReflProb::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/ppt/light/reflprob.shader");
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

	Utl_CreatePipeline(gpuShader, colorBlending);
}

void PtReflProb::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, vk::DescriptorSet gbufferDescSet)
{
	auto camera = sceneDesc.viewer;

	if (!camera) {
		return;
	}

	// WIP:
	auto descSet = camera->descSets[sceneDesc.frameIndex];

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, 1u, &gbufferDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 1u, 1u, &descSet, 0u, nullptr);

	for (auto rp : sceneDesc->reflProbs.elements) {
		if (rp) {
			cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 2u, 1u,
				&rp->envmap.Lock().descriptorSet, 0u, nullptr);

			// draw call (triangle)
			cmdBuffer.draw(3u, 1u, 0u, 0u);
		}
	}
}
} // namespace vl
