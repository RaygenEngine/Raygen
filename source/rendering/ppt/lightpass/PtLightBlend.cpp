#include "pch.h"
#include "PtLightBlend.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Device.h"
#include "rendering/Renderer.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneSpotlight.h"
#include "rendering/structures/Depthmap.h"
#include "rendering/scene/SceneCamera.h"

namespace vl {
void PtLightBlend::MakeLayout()
{
	std::array layouts{
		Layouts->renderAttachmentsLayout.handle(),
	};

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.setSetLayouts(layouts);

	m_pipelineLayout = Device->createPipelineLayoutUnique(pipelineLayoutInfo);
}

void PtLightBlend::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/lightblend.shader");
	gpuShader.onCompile = [&]() {
		MakePipeline();
	};

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment
		.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
						   | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) //
		.setBlendEnable(VK_FALSE);

	vk::PipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending
		.setLogicOpEnable(VK_FALSE) //
		.setLogicOp(vk::LogicOp::eCopy)
		.setAttachments(colorBlendAttachment)
		.setBlendConstants({ 0.f, 0.f, 0.f, 0.f });


	Utl_CreatePipeline(gpuShader, colorBlending);
}

void PtLightBlend::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	auto camera = sceneDesc.viewer;

	if (!camera) {
		return;
	}

	// WIP:
	auto descSet = camera->descSet[sceneDesc.frameIndex];

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, 1u, &sceneDesc.attDesc, 0u, nullptr);

	// draw call (triangle)
	cmdBuffer.draw(3u, 1u, 0u, 0u);
}

} // namespace vl
