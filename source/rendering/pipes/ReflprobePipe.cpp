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
	return rvk::makePipelineLayoutEx(
		{
			DescriptorLayouts->global.handle(),
			PassLayouts->main.internalDescLayout.handle(),
			DescriptorLayouts->_1uniformBuffer.handle(),
			DescriptorLayouts->_1imageSampler.handle(),
			DescriptorLayouts->_1imageSampler.handle(),
			DescriptorLayouts->_1imageSampler.handle(),
		},
		vk::ShaderStageFlagBits::eVertex, sizeof(PushConstant));
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
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo
		.setVertexBindingDescriptions(bindingDescription) //
		.setVertexAttributeDescriptions(attributeDescription);

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly
		.setTopology(vk::PrimitiveTopology::eTriangleList) //
		.setPrimitiveRestartEnable(VK_FALSE);

	// those are dynamic so they will be updated when needed
	vk::Viewport viewport{};
	vk::Rect2D scissor{};

	vk::PipelineViewportStateCreateInfo viewportState{};
	viewportState
		.setViewportCount(1u) //
		.setPViewports(&viewport)
		.setScissorCount(1u)
		.setPScissors(&scissor);

	vk::PipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer
		.setDepthClampEnable(VK_FALSE) //
		.setRasterizerDiscardEnable(VK_FALSE)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setLineWidth(1.f)
		.setCullMode(vk::CullModeFlagBits::eBack)
		.setFrontFace(vk::FrontFace::eClockwise)
		.setDepthBiasEnable(VK_FALSE)
		.setDepthBiasConstantFactor(0.f)
		.setDepthBiasClamp(0.f)
		.setDepthBiasSlopeFactor(0.f);

	vk::PipelineMultisampleStateCreateInfo multisampling{};
	multisampling
		.setSampleShadingEnable(VK_FALSE) //
		.setRasterizationSamples(vk::SampleCountFlagBits::e1)
		.setMinSampleShading(1.f)
		.setPSampleMask(nullptr)
		.setAlphaToCoverageEnable(VK_FALSE)
		.setAlphaToOneEnable(VK_FALSE);

	// dynamic states
	std::array dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,
	};
	vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
	dynamicStateInfo.setDynamicStates(dynamicStates);

	// depth and stencil state
	vk::PipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil
		.setDepthTestEnable(VK_FALSE) //
		.setDepthWriteEnable(VK_FALSE)
		.setDepthCompareOp(vk::CompareOp::eLess)
		.setDepthBoundsTestEnable(VK_FALSE)
		.setMinDepthBounds(0.0f) // Optional
		.setMaxDepthBounds(1.0f) // Optional
		.setStencilTestEnable(VK_FALSE)
		.setFront({}) // Optional
		.setBack({}); // Optional

	vk::GraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo
		.setStageCount(static_cast<uint32>(gpuShader.shaderStages.size())) //
		.setPStages(gpuShader.shaderStages.data())
		.setPVertexInputState(&vertexInputInfo)
		.setPInputAssemblyState(&inputAssembly)
		.setPViewportState(&viewportState)
		.setPRasterizationState(&rasterizer)
		.setPMultisampleState(&multisampling)
		.setPDepthStencilState(&depthStencil)
		.setPColorBlendState(&colorBlending)
		.setPDynamicState(&dynamicStateInfo)
		.setLayout(layout())
		.setRenderPass(*PassLayouts->main.compatibleRenderPass)
		.setSubpass(2u)
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	return Device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
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
			sceneDesc.viewer.ubo.viewProj * math::transformMat(rp->ubo.radius, rp->ubo.position),
		};

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		rvk::drawSphere18x9(cmdBuffer);
	}
}

} // namespace vl
