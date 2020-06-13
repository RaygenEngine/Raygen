#include "pch.h"
#include "GBufferPass.h"

#include "engine/Engine.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Device.h"
#include "rendering/Renderer.h"
#include "rendering/scene/Scene.h"
#include "rendering/Layouts.h"
#include "rendering/objects/GBuffer.h"

#include <glm/gtc/matrix_inverse.hpp>

namespace {
struct PushConstant {
	glm::mat4 modelMat;
	glm::mat4 normalMat;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
vk::UniqueRenderPass GBufferPass::CreateCompatibleRenderPass()
{
	// renderpass
	std::array<vk::AttachmentDescription, 5> colorAttachmentDescs{};
	std::array<vk::AttachmentReference, 5> colorAttachmentRefs{};

	for (size_t i = 0; i < 5; ++i) {
		colorAttachmentDescs[i]
			.setFormat(GBuffer::colorAttachmentFormats[i]) // CHECK:
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(
				vk::ImageLayout::eColorAttachmentOptimal)             // CHECK: vk::ImageLayout::eShaderReadOnlyOptimal?
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // CHECK:

		colorAttachmentRefs[i]
			.setAttachment(static_cast<uint32>(i)) //
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	}

	vk::AttachmentDescription depthAttachmentDesc{};
	depthAttachmentDesc
		.setFormat(Device->pd->FindDepthFormat()) // CHECK:
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare) // CHECK: if use stencil dont forget those two
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(
			vk::ImageLayout::eDepthStencilAttachmentOptimal)      // CHECK: vk::ImageLayout::eShaderReadOnlyOptimal?
		.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // CHECK:

	vk::AttachmentReference depthAttachmentRef{};
	depthAttachmentRef
		.setAttachment(5u) //
		.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass{};
	subpass
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics) //
		.setColorAttachmentCount(static_cast<uint32>(colorAttachmentRefs.size()))
		.setPColorAttachments(colorAttachmentRefs.data())
		.setPDepthStencilAttachment(&depthAttachmentRef);

	vk::SubpassDependency dependency{};
	dependency
		.setSrcSubpass(VK_SUBPASS_EXTERNAL) //
		.setDstSubpass(0u)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlags(0)) // 0
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	std::array attachments{ colorAttachmentDescs[0], colorAttachmentDescs[1], colorAttachmentDescs[2],
		colorAttachmentDescs[3], colorAttachmentDescs[4], depthAttachmentDesc };
	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo
		.setAttachmentCount(static_cast<uint32>(attachments.size())) //
		.setPAttachments(attachments.data())
		.setSubpassCount(1u)
		.setPSubpasses(&subpass)
		.setDependencyCount(1u)
		.setPDependencies(&dependency);

	return Device->createRenderPassUnique(renderPassInfo);
}

vk::UniquePipeline GBufferPass::CreatePipeline(
	vk::PipelineLayout pipelineLayout, std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages)
{
	vk::VertexInputBindingDescription bindingDescription{};
	bindingDescription
		.setBinding(0u) //
		.setStride(sizeof(Vertex))
		.setInputRate(vk::VertexInputRate::eVertex);

	std::array<vk::VertexInputAttributeDescription, 5> attributeDescriptions{};

	attributeDescriptions[0].binding = 0u;
	attributeDescriptions[0].location = 0u;
	attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[0].offset = offsetof(Vertex, position);

	attributeDescriptions[1].binding = 0u;
	attributeDescriptions[1].location = 1u;
	attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[1].offset = offsetof(Vertex, normal);

	attributeDescriptions[2].binding = 0u;
	attributeDescriptions[2].location = 2u;
	attributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[2].offset = offsetof(Vertex, tangent);

	attributeDescriptions[3].binding = 0u;
	attributeDescriptions[3].location = 3u;
	attributeDescriptions[3].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[3].offset = offsetof(Vertex, bitangent);

	attributeDescriptions[4].binding = 0u;
	attributeDescriptions[4].location = 4u;
	attributeDescriptions[4].format = vk::Format::eR32G32Sfloat;
	attributeDescriptions[4].offset = offsetof(Vertex, uv);

	// fixed-function stage
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo
		.setVertexBindingDescriptionCount(1u) //
		.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()))
		.setPVertexBindingDescriptions(&bindingDescription)
		.setPVertexAttributeDescriptions(attributeDescriptions.data());


	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly
		.setTopology(vk::PrimitiveTopology::eTriangleList) //
		.setPrimitiveRestartEnable(VK_FALSE);

	// Dynamic vieport
	vk::DynamicState dynamicStates[2] = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
	dynamicStateInfo
		.setDynamicStateCount(2u) //
		.setPDynamicStates(&dynamicStates[0]);


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
		.setCullMode(static_cast<vk::CullModeFlags>(vk::CullModeFlagBits::eBack))
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

	std::array<vk::PipelineColorBlendAttachmentState, 5> colorBlendAttachment{};
	for (uint32 i = 0u; i < 5; ++i) {
		colorBlendAttachment[i]
			.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
							   | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) //
			.setBlendEnable(VK_FALSE)
			.setSrcColorBlendFactor(vk::BlendFactor::eOne)
			.setDstColorBlendFactor(vk::BlendFactor::eZero)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
			.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
			.setAlphaBlendOp(vk::BlendOp::eAdd);
	}

	vk::PipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending
		.setLogicOpEnable(VK_FALSE) //
		.setLogicOp(vk::LogicOp::eCopy)
		.setAttachmentCount(static_cast<uint32>(colorBlendAttachment.size()))
		.setPAttachments(colorBlendAttachment.data())
		.setBlendConstants({ 0.f, 0.f, 0.f, 0.f });


	// depth and stencil state
	vk::PipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil
		.setDepthTestEnable(VK_TRUE) //
		.setDepthWriteEnable(VK_TRUE)
		.setDepthCompareOp(vk::CompareOp::eLess)
		.setDepthBoundsTestEnable(VK_FALSE)
		.setMinDepthBounds(0.0f) // Optional
		.setMaxDepthBounds(1.0f) // Optional
		.setStencilTestEnable(VK_FALSE)
		.setFront({}) // Optional
		.setBack({}); // Optional

	vk::GraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo
		.setStageCount(static_cast<uint32>(shaderStages.size())) //
		.setPStages(shaderStages.data())
		.setPVertexInputState(&vertexInputInfo)
		.setPInputAssemblyState(&inputAssembly)
		.setPViewportState(&viewportState)
		.setPRasterizationState(&rasterizer)
		.setPMultisampleState(&multisampling)
		.setPDepthStencilState(&depthStencil)
		.setPColorBlendState(&colorBlending)
		.setPDynamicState(&dynamicStateInfo)
		.setLayout(pipelineLayout)
		.setRenderPass(Layouts->gbufferPass.get())
		.setSubpass(0u)
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	return Device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
}

void GBufferPass::RecordCmd(
	vk::CommandBuffer* cmdBuffer, GBuffer* gBuffer, const std::vector<SceneGeometry*>& geometries)
{
	PROFILE_SCOPE(Renderer);

	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo
		.setRenderPass(Layouts->gbufferPass.get()) //
		.setFramebuffer(gBuffer->framebuffer.get());
	renderPassInfo.renderArea
		.setOffset({ 0, 0 }) //
		.setExtent(gBuffer->attachments[GPosition]->GetExtent2D());

	std::array<vk::ClearValue, 6> clearValues = {};
	clearValues[0].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[1].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[2].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[3].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[4].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[5].setDepthStencil({ 1.0f, 0 });
	renderPassInfo
		.setClearValueCount(static_cast<uint32>(clearValues.size())) //
		.setPClearValues(clearValues.data());


	cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	{
		// TODO: remove camera refs
		auto camera = Scene->GetActiveCamera();
		if (!camera) {
			cmdBuffer->endRenderPass();
			return;
		}

		for (auto geom : geometries) {
			if (!geom) {
				continue;
			}
			PushConstant pc{ //
				geom->transform, glm::inverseTranspose(glm::mat3(geom->transform))
			};

			for (auto& gg : geom->model.Lock().geometryGroups) {
				auto& mat = gg.material.Lock();

				cmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *mat.pipeline);
				cmdBuffer->pushConstants(
					*mat.plLayout, vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

				if (mat.hasDescriptorSet) {
					cmdBuffer->bindDescriptorSets(
						vk::PipelineBindPoint::eGraphics, *mat.plLayout, 0u, 1u, &mat.descSet, 0u, nullptr);
				}

				cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *mat.plLayout, 1u, 1u,
					&Scene->GetActiveCameraDescSet(), 0u, nullptr);

				cmdBuffer->bindVertexBuffers(0u, { *gg.vertexBuffer }, { 0 });
				cmdBuffer->bindIndexBuffer(*gg.indexBuffer, 0, vk::IndexType::eUint32);


				cmdBuffer->drawIndexed(gg.indexCount, 1u, 0u, 0u, 0u);
			}
		}
	}
	cmdBuffer->endRenderPass();
}

} // namespace vl
