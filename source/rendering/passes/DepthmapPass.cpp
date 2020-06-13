#include "pch.h"
#include "DepthmapPass.h"

#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"

namespace {
struct PushConstant {
	glm::mat4 mvp;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
vk::UniqueRenderPass DepthmapPass::CreateCompatibleRenderPass()
{
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
		.setAttachment(0u) //
		.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass{};
	subpass
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics) //
		.setColorAttachmentCount(0u)
		.setPColorAttachments(nullptr)
		.setPDepthStencilAttachment(&depthAttachmentRef);

	vk::SubpassDependency dependency{};
	dependency
		.setSrcSubpass(VK_SUBPASS_EXTERNAL) //
		.setDstSubpass(0u)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlags(0)) // 0
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	std::array attachments{ depthAttachmentDesc };
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


vk::UniquePipeline DepthmapPass::CreatePipeline(
	vk::PipelineLayout pipelineLayout, std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages)
{
	// fixed-function stage
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

	vk::VertexInputBindingDescription bindingDescription{};
	bindingDescription
		.setBinding(0u) //
		.setStride(sizeof(Vertex))
		.setInputRate(vk::VertexInputRate::eVertex);

	std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};

	attributeDescriptions[0].binding = 0u;
	attributeDescriptions[0].location = 0u;
	attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[0].offset = offsetof(Vertex, position);

	attributeDescriptions[1].binding = 0u;
	attributeDescriptions[1].location = 1u;
	attributeDescriptions[1].format = vk::Format::eR32G32Sfloat;
	attributeDescriptions[1].offset = offsetof(Vertex, uv);

	vertexInputInfo
		.setVertexBindingDescriptionCount(1u) //
		.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()))
		.setPVertexBindingDescriptions(&bindingDescription)
		.setPVertexAttributeDescriptions(attributeDescriptions.data());


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

	// Dynamic vieport
	vk::DynamicState dynamicStates[2] = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
	dynamicStateInfo
		.setDynamicStateCount(2u) //
		.setPDynamicStates(&dynamicStates[0]);

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
		.setPColorBlendState(nullptr)
		.setPDynamicState(&dynamicStateInfo)
		.setLayout(pipelineLayout)
		.setRenderPass(Layouts->depthRenderPass.get())
		.setSubpass(0u)
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	return Device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
}

void DepthmapPass::RecordCmd(vk::CommandBuffer* cmdBuffer, RDepthmap& depthmap, const glm::mat4& viewProj,
	const std::vector<SceneGeometry*>& geometries)
{
	PROFILE_SCOPE(Renderer);

	auto extent = depthmap.attachment->GetExtent2D();

	vk::Rect2D scissor{};

	scissor
		.setOffset({ 0, 0 }) //
		.setExtent(extent);

	auto vpSize = extent;

	vk::Viewport viewport{};
	viewport
		.setX(0) //
		.setY(0)
		.setWidth(static_cast<float>(vpSize.width))
		.setHeight(static_cast<float>(vpSize.height))
		.setMinDepth(0.f)
		.setMaxDepth(1.f);

	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo
		.setRenderPass(Layouts->depthRenderPass.get()) //
		.setFramebuffer(depthmap.framebuffer.get());
	renderPassInfo.renderArea
		.setOffset({ 0, 0 }) //
		.setExtent(extent);

	vk::ClearValue clearValues = {};
	clearValues.setDepthStencil({ 1.0f, 0 });
	renderPassInfo
		.setClearValueCount(1u) //
		.setPClearValues(&clearValues);

	// begin render pass
	cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	{

		cmdBuffer->setViewport(0, { viewport });
		cmdBuffer->setScissor(0, { scissor });

		for (auto geom : geometries) {
			if (!geom) {
				continue;
			}

			PushConstant pc{ //
				viewProj * geom->transform
			};

			for (auto& gg : geom->model.Lock().geometryGroups) {
				auto& mat = gg.material.Lock();

				vk::Buffer vertexBuffers[] = { *gg.vertexBuffer };
				vk::DeviceSize offsets[] = { 0 };
				// bind the graphics pipeline
				cmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, mat.depthPipeline.get());

				// Submit via push constant (rather than a UBO)
				cmdBuffer->pushConstants(
					mat.depthPlLayout.get(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

				// geom
				cmdBuffer->bindVertexBuffers(0u, 1u, vertexBuffers, offsets);

				// indices
				cmdBuffer->bindIndexBuffer(*gg.indexBuffer, 0, vk::IndexType::eUint32);


				if (mat.hasDescriptorSet) {
					// descriptor sets
					cmdBuffer->bindDescriptorSets(
						vk::PipelineBindPoint::eGraphics, mat.depthPlLayout.get(), 0u, 1u, &mat.descSet, 0u, nullptr);
				}

				cmdBuffer->drawIndexed(gg.indexCount, 1u, 0u, 0u, 0u);
			}
		}
	}
	// end render pass
	cmdBuffer->endRenderPass();
}
} // namespace vl
