#include "PointlightPipe.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/ScenePointlight.h"

namespace {
struct PushConstant {
	glm::mat4 lightVolMatVP;
};

static_assert(sizeof(PushConstant) <= 128);

} // namespace


namespace vl {
vk::UniquePipelineLayout PointlightPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayout<PushConstant>(
		{
			DescriptorLayouts->global.handle(),
			PassLayouts->main.internalDescLayout.handle(),
			DescriptorLayouts->_1uniformBuffer.handle(),
			DescriptorLayouts->accelerationStructure.handle(),
		},
		vk::ShaderStageFlagBits::eVertex);
}

vk::UniquePipeline PointlightPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/lighting/pointlight.shader");
	gpuShader.onCompile = [&]() {
		StaticPipes::Recompile<PointlightPipe>();
	};

	// additive blending
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

	vk::PipelineVertexInputStateCreateInfo vertexInputState{};
	vertexInputState
		.setVertexBindingDescriptions(bindingDescription) //
		.setVertexAttributeDescriptions(attributeDescription);

	vk::PipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer
		.setDepthClampEnable(VK_FALSE) //
		.setRasterizerDiscardEnable(VK_FALSE)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setLineWidth(1.f)
		.setCullMode(vk::CullModeFlagBits::eFront)
		.setFrontFace(vk::FrontFace::eCounterClockwise)
		.setDepthBiasEnable(VK_FALSE)
		.setDepthBiasConstantFactor(0.f)
		.setDepthBiasClamp(0.f)
		.setDepthBiasSlopeFactor(0.f);


	return rvk::makeGraphicsPipeline(gpuShader.shaderStages, &vertexInputState, nullptr, nullptr, nullptr, &rasterizer,
		nullptr, nullptr, &colorBlending, nullptr, layout(), PassLayouts->main.compatibleRenderPass.get(), 1u);
}

void PointlightPipe::RecordCmd(
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

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics, layout(), 3u, { sceneDesc.scene->sceneAsDescSet }, nullptr);

	// bind unit sphere once
	rvk::bindSphere18x9(cmdBuffer);

	for (auto pl : sceneDesc->Get<ScenePointlight>()) {
		PushConstant pc{ sceneDesc.viewer.ubo.viewProj * pl->volumeTransform };

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		cmdBuffer.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics, layout(), 2u, { pl->uboDescSet[sceneDesc.frameIndex] }, nullptr);

		rvk::drawSphere18x9(cmdBuffer);
	}
}

} // namespace vl
