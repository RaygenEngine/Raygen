#include "ReflprobePipe.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/SceneReflProbe.h"

namespace {
struct PushConstant {
	glm::mat4 volumeMat;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace


namespace vl {
vk::UniquePipelineLayout ReflprobePipe::MakePipelineLayout()
{
	return rvk::makePipelineLayout<PushConstant>(
		{
			DescriptorLayouts->global.handle(),
			PassLayouts->main.internalDescLayout.handle(),
			DescriptorLayouts->_1uniformBuffer.handle(),
			DescriptorLayouts->_1imageSampler.handle(),
			DescriptorLayouts->_1imageSampler.handle(),
			DescriptorLayouts->_1imageSampler.handle(),
		},
		vk::ShaderStageFlagBits::eVertex);
}

vk::UniquePipeline ReflprobePipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/lighting/reflprobe.shader");
	gpuShader.onCompile = [&]() {
		StaticPipes::Recompile<ReflprobePipe>();
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

	vk::VertexInputBindingDescription bindingDescription{};
	bindingDescription
		.setBinding(0u) //
		.setStride(sizeof(glm::vec3))
		.setInputRate(vk::VertexInputRate::eVertex);

	vk::VertexInputAttributeDescription attributeDescription{};

	attributeDescription.binding = 0u;
	attributeDescription.location = 0u;
	attributeDescription.format = vk::Format::eR32G32B32Sfloat;
	attributeDescription.offset = 0u;

	// fixed-function stage
	vk::PipelineVertexInputStateCreateInfo vertexInputState{};
	vertexInputState
		.setVertexBindingDescriptions(bindingDescription) //
		.setVertexAttributeDescriptions(attributeDescription);


	return rvk::makeGraphicsPipeline(gpuShader.shaderStages, &vertexInputState, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, &colorBlending, nullptr, layout(), PassLayouts->main.compatibleRenderPass.get(), 2u);
}

void ReflprobePipe::RecordCmd(
	vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, vk::DescriptorSet inputDescSet) const
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout(), 0u,
		{
			sceneDesc.globalDesc,
			inputDescSet,
		},
		nullptr);

	// bind unit sphere once
	rvk::bindSphere18x9(cmdBuffer);

	for (auto rp : sceneDesc->Get<SceneReflprobe>()) {

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout(), 2u,
			{
				rp->uboDescSet[sceneDesc.frameIndex],
				rp->environmentSamplerDescSet,
				rp->irradianceSamplerDescSet,
				rp->prefilteredSamplerDescSet,
			},
			nullptr);

		PushConstant pc{
			.volumeMat = sceneDesc.viewer.ubo.viewProj * math::transformMat(rp->ubo.radius, rp->ubo.position),
		};

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		rvk::drawSphere18x9(cmdBuffer);
	}
}

} // namespace vl
