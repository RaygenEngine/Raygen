#include "PrefilteredMapCalculation.h"

#include "engine/console/ConsoleVariable.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Device.h"
#include "rendering/scene/SceneReflProbe.h"

namespace {
struct PushConstant {
	glm::mat4 rotVp;
	float a;
};

static_assert(sizeof(PushConstant) <= 128);

std::array vertices = { // positions
	-1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
	-1.0f,

	-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
	1.0f,

	1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,

	-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
	1.0f
};
} // namespace


namespace vl {
PrefilteredMapCalculation::PrefilteredMapCalculation()
{
	m_cubeVertexBuffer = RBuffer::CreateTransfer("Cube Vertex", vertices, vk::BufferUsageFlagBits::eVertexBuffer);
}

void PrefilteredMapCalculation::RecordPass(vk::CommandBuffer cmdBuffer, const SceneReflprobe& rp) const
{
	uint32 resolution = rp.prefiltered.extent.width;

	auto projInverse = glm::perspective(glm::radians(90.0f), 1.f, 1.f, 25.f);
	projInverse[1][1] *= -1;

	std::array viewMats{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)),   // right
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)),  // left
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)),   // up
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)), // down
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0)),  // front
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0)),   // back
	};

	// for each mip / framebuffer / face
	for (int32 mip = 0; mip < rp.ubo.lodCount; ++mip) {
		for (uint32 i = 0; i < 6; ++i) {

			uint32 mipResolution = static_cast<uint32>(resolution * std::pow(0.5, mip));

			vk::Rect2D scissor{};

			scissor
				.setOffset({ 0, 0 }) //
				.setExtent({ mipResolution, mipResolution });

			vk::Viewport viewport{};

			viewport
				.setWidth(static_cast<float>(mipResolution)) //
				.setHeight(static_cast<float>(mipResolution));


			vk::RenderPassBeginInfo renderPassInfo{};
			renderPassInfo
				.setRenderPass(Layouts->singleFloatColorAttPassLayout.compatibleRenderPass.get()) //
				.setFramebuffer(rp.pref_cubemapMips[mip].framebuffers[i].get());
			renderPassInfo.renderArea
				.setOffset({ 0, 0 }) //
				.setExtent(scissor.extent);

			std::array<vk::ClearValue, 1> clearValues{};
			clearValues[0].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
			renderPassInfo.setClearValues(clearValues);

			// begin render pass
			cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
			{
				// Dynamic viewport & scissor
				cmdBuffer.setViewport(0, { viewport });
				cmdBuffer.setScissor(0, { scissor });

				// bind the graphics pipeline
				cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline());


				float roughness = (float)mip / (float)(rp.ubo.lodCount - 1);
				float a = roughness * roughness;
				LOG_DEBUG("Prefiltered mip a = roughness = {}", a);

				PushConstant pc{
					//
					projInverse * glm::mat4(glm::mat3(viewMats[i])),
					a,
				};

				// Submit via push constant (rather than a UBO)
				cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
					0u, sizeof(PushConstant), &pc);

				// geom
				cmdBuffer.bindVertexBuffers(0u, { m_cubeVertexBuffer.handle() }, { vk::DeviceSize(0) });

				// descriptor sets
				cmdBuffer.bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics, layout(), 0u, rp.surroundingEnvSamplerDescSet, nullptr);

				// draw call (cube)
				cmdBuffer.draw(static_cast<uint32>(vertices.size() / 3), 1u, 0u, 0u);
			}
			// end render pass
			cmdBuffer.endRenderPass();
		}
	}
}


vk::UniquePipelineLayout PrefilteredMapCalculation::MakePipelineLayout()
{
	// pipeline layout
	vk::PushConstantRange pushConstantRange{};
	pushConstantRange
		.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment) //
		.setSize(sizeof(PushConstant))
		.setOffset(0u);

	std::array layouts{ Layouts->cubemapLayout.handle() };
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setSetLayouts(layouts) //
		.setPushConstantRanges(pushConstantRange);

	return Device->createPipelineLayoutUnique(pipelineLayoutInfo);
}

vk::UniquePipeline PrefilteredMapCalculation::MakePipeline()
{
	auto& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/offline/prefiltered.shader");

	if (!gpuShader.HasValidModule()) {
		LOG_ERROR("Pref Pipeline skipped due to shader compilation errors.");
		return {};
	}
	std::vector shaderStages = gpuShader.shaderStages;

	auto& fragShaderModule = gpuShader.frag;
	auto& vertShaderModule = gpuShader.vert;

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

	vk::VertexInputBindingDescription bindingDescription{};
	bindingDescription
		.setBinding(0u) //
		.setStride(sizeof(float) * 3)
		.setInputRate(vk::VertexInputRate::eVertex);

	vk::VertexInputAttributeDescription attributeDescription{};

	attributeDescription
		.setBinding(0u) //
		.setLocation(0u)
		.setFormat(vk::Format::eR32G32B32Sfloat)
		.setOffset(0u);

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
		.setViewports(viewport) //
		.setScissors(scissor);

	static ConsoleVariable<uint> fillmode{ "fillmode", 0 };

	vk::PipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer
		.setDepthClampEnable(VK_FALSE) //
		.setRasterizerDiscardEnable(VK_FALSE)
		.setPolygonMode(static_cast<vk::PolygonMode>(fillmode.Get()))
		.setLineWidth(1.f)
		.setCullMode(vk::CullModeFlagBits::eBack)
		.setFrontFace(vk::FrontFace::eCounterClockwise)
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

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{};

	colorBlendAttachment
		.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
						   | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) //
		.setBlendEnable(VK_FALSE)
		.setSrcColorBlendFactor(vk::BlendFactor::eOne)
		.setDstColorBlendFactor(vk::BlendFactor::eZero)
		.setColorBlendOp(vk::BlendOp::eAdd)
		.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
		.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
		.setAlphaBlendOp(vk::BlendOp::eAdd);


	vk::PipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending
		.setLogicOpEnable(VK_FALSE) //
		.setLogicOp(vk::LogicOp::eCopy)
		.setAttachments(colorBlendAttachment)
		.setBlendConstants({ 0.f, 0.f, 0.f, 0.f });

	// Dynamic vieport
	std::array dynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
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
		.setStages(shaderStages) //
		.setPVertexInputState(&vertexInputInfo)
		.setPInputAssemblyState(&inputAssembly)
		.setPViewportState(&viewportState)
		.setPRasterizationState(&rasterizer)
		.setPMultisampleState(&multisampling)
		.setPDepthStencilState(&depthStencil)
		.setPColorBlendState(&colorBlending)
		.setPDynamicState(&dynamicStateInfo)
		.setLayout(layout())
		.setRenderPass(Layouts->singleFloatColorAttPassLayout.compatibleRenderPass.get())
		.setSubpass(0u)
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	return Device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
}
} // namespace vl
