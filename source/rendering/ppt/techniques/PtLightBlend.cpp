#include "PtLightBlend.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/scene/SceneCamera.h"


namespace {
struct PushConstant {
	int32 quadlightCount;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
void PtLightBlend::MakeLayout()
{
	m_pipelineLayout = rvk::makePipelineLayoutEx(
		{
			DescriptorLayouts->global.handle(),
			DescriptorLayouts->_1storageBuffer.handle(), // quadlights
			DescriptorLayouts->accelerationStructure.handle(),
		},
		vk::ShaderStageFlagBits::eFragment, sizeof(PushConstant));
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

void PtLightBlend::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u,
		{
			sceneDesc.globalDesc,
			sceneDesc.scene->tlas.sceneDesc.descSetQuadlights[sceneDesc.frameIndex],
			sceneDesc.scene->sceneAsDescSet,
		},
		nullptr);

	PushConstant pc{
		sceneDesc.scene->tlas.sceneDesc.quadlightCount,
	};

	cmdBuffer.pushConstants(m_pipelineLayout.get(), vk::ShaderStageFlagBits::eFragment, 0u, sizeof(PushConstant), &pc);

	// big triangle
	cmdBuffer.draw(3u, 1u, 0u, 0u);
}

} // namespace vl
