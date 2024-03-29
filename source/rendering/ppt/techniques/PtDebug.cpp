#include "PtDebug.h"

#include "rendering/Layouts.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"

namespace vl {
void PtDebug::MakeLayout()
{
	m_pipelineLayout = rvk::makePipelineLayoutNoPC({
		PassLayouts->postproc.internalDescLayout.handle(),
	});
}

void PtDebug::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/ppt/ppt_debug.shader");
	gpuShader.onCompile = [&]() {
		MakePipeline();
	};

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment
		.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
						   | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) //
		.setBlendEnable(VK_FALSE)
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

	Utl_CreatePipeline(gpuShader, colorBlending, 1u);
}

void PtDebug::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	cmdBuffer.nextSubpass(vk::SubpassContents::eInline);
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());

	// cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, TODO:
	//	{ Renderer->m_ptPass[sceneDesc.frameIndex].internalDescSet }, nullptr);

	// big triangle
	cmdBuffer.draw(3u, 1u, 0u, 0u);
}
} // namespace vl
