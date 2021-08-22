#include "PtLightBlend.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/Device.h"


namespace {
struct PushConstant {
	int32 quadlightCount;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
void PtLightBlend::MakeLayout()
{
	std::array layouts{
		Layouts->globalDescLayout.handle(),
		Layouts->singleStorageBuffer.handle(), // quadlights
		Layouts->accelLayout.handle(),
	};

	// pipeline layout
	vk::PushConstantRange pushConstantRange{};
	pushConstantRange
		.setStageFlags(vk::ShaderStageFlagBits::eFragment) //
		.setSize(sizeof(PushConstant))
		.setOffset(0u);


	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setPushConstantRanges(pushConstantRange) //
		.setSetLayouts(layouts);

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
