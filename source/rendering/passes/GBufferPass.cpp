#include "pch.h"
#include "GbufferPass.h"

#include "engine/Engine.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuMaterialArchetype.h"
#include "rendering/assets/GpuMaterialInstance.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/assets/GpuSkinnedMesh.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/Renderer.h"
#include "rendering/scene/Scene.h"
#include "rendering/structures/GBuffer.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/SceneGeometry.h"

#include <glm/gtc/matrix_inverse.hpp>

#include "assets/shared/GeometryShared.h"

namespace {
struct PushConstant {
	glm::mat4 modelMat;
	glm::mat4 normalMat;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
vk::UniqueRenderPass GbufferPass::CreateCompatibleRenderPass()
{
	// renderpass
	std::array<vk::AttachmentDescription, GBuffer::ColorAttachmentCount> colorAttachmentDescs{};
	std::array<vk::AttachmentReference, GBuffer::ColorAttachmentCount> colorAttachmentRefs{};

	for (size_t i = 0; i < GBuffer::ColorAttachmentCount; ++i) {
		colorAttachmentDescs[i]
			.setFormat(GBuffer::colorAttachmentFormats[i]) //
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		colorAttachmentRefs[i]
			.setAttachment(static_cast<uint32>(i)) //
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	}

	vk::AttachmentDescription depthAttachmentDesc{};
	depthAttachmentDesc
		.setFormat(Device->FindDepthFormat()) //
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
		.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	vk::AttachmentReference depthAttachmentRef{};
	depthAttachmentRef
		.setAttachment(4u) //
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

	std::vector<vk::AttachmentDescription> attachments{ colorAttachmentDescs.begin(), colorAttachmentDescs.end() };
	attachments.push_back(depthAttachmentDesc);

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

size_t GbufferPass::GetPushConstantSize()
{
	return sizeof(PushConstant);
}

namespace {
	vk::UniquePipeline CreatePipelineFromVtxInfo(vk::PipelineLayout pipelineLayout,
		std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages,
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo)
	{
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

		std::array<vk::PipelineColorBlendAttachmentState, GBuffer::ColorAttachmentCount> colorBlendAttachment{};
		for (uint32 i = 0u; i < GBuffer::ColorAttachmentCount; ++i) {
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
} // namespace

vk::UniquePipeline GbufferPass::CreatePipeline(
	vk::PipelineLayout pipelineLayout, std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages)
{
	vk::VertexInputBindingDescription bindingDescription{};
	bindingDescription
		.setBinding(0u) //
		.setStride(sizeof(Vertex))
		.setInputRate(vk::VertexInputRate::eVertex);

	std::array<vk::VertexInputAttributeDescription, 4> attributeDescriptions{};

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
	attributeDescriptions[3].format = vk::Format::eR32G32Sfloat;
	attributeDescriptions[3].offset = offsetof(Vertex, uv);

	// fixed-function stage
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo
		.setVertexBindingDescriptionCount(1u) //
		.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()))
		.setPVertexBindingDescriptions(&bindingDescription)
		.setPVertexAttributeDescriptions(attributeDescriptions.data());

	return CreatePipelineFromVtxInfo(pipelineLayout, shaderStages, vertexInputInfo);
}

vk::UniquePipeline GbufferPass::CreateAnimPipeline(
	vk::PipelineLayout pipelineLayout, std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages)
{
	vk::VertexInputBindingDescription bindingDescription{};
	bindingDescription
		.setBinding(0u) //
		.setStride(sizeof(SkinnedVertex))
		.setInputRate(vk::VertexInputRate::eVertex);

	std::array<vk::VertexInputAttributeDescription, 6> attributeDescriptions{};

	attributeDescriptions[0].binding = 0u;
	attributeDescriptions[0].location = 0u;
	attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[0].offset = offsetof(SkinnedVertex, position);

	attributeDescriptions[1].binding = 0u;
	attributeDescriptions[1].location = 1u;
	attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[1].offset = offsetof(SkinnedVertex, normal);

	attributeDescriptions[2].binding = 0u;
	attributeDescriptions[2].location = 2u;
	attributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[2].offset = offsetof(SkinnedVertex, tangent);

	attributeDescriptions[3].binding = 0u;
	attributeDescriptions[3].location = 3u;
	attributeDescriptions[3].format = vk::Format::eR32G32Sfloat;
	attributeDescriptions[3].offset = offsetof(SkinnedVertex, uv);

	attributeDescriptions[4].binding = 0u;
	attributeDescriptions[4].location = 4u;
	attributeDescriptions[4].format = vk::Format::eR32G32B32A32Sint;
	attributeDescriptions[4].offset = offsetof(SkinnedVertex, joint);

	attributeDescriptions[5].binding = 0u;
	attributeDescriptions[5].location = 5u;
	attributeDescriptions[5].format = vk::Format::eR32G32B32A32Sfloat;
	attributeDescriptions[5].offset = offsetof(SkinnedVertex, weight);

	// fixed-function stage
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo
		.setVertexBindingDescriptionCount(1u) //
		.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()))
		.setPVertexBindingDescriptions(&bindingDescription)
		.setPVertexAttributeDescriptions(attributeDescriptions.data());

	return CreatePipelineFromVtxInfo(pipelineLayout, shaderStages, vertexInputInfo);
}

void GbufferPass::RecordCmd(
	vk::CommandBuffer* cmdBuffer, vk::Viewport viewport, vk::Rect2D scissor, const SceneRenderDesc& sceneDesc)
{
	// PROFILE_SCOPE(Renderer);

	vk::CommandBufferInheritanceInfo ii{};
	ii.setRenderPass(Layouts->gbufferPass.get()) //
		.setFramebuffer({})                      // VK_NULL_HANDLE
		.setSubpass(0u);

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo
		.setFlags(vk::CommandBufferUsageFlagBits::eRenderPassContinue) //
		.setPInheritanceInfo(&ii);

	cmdBuffer->begin(beginInfo);
	{

		cmdBuffer->setViewport(0, { viewport });
		cmdBuffer->setScissor(0, { scissor });

		auto camera = sceneDesc.viewer;
		if (!camera) {
			cmdBuffer->endRenderPass();
			return;
		}

		// WIP: decouple
		auto descSet = camera->descSet[sceneDesc.frameIndex];

		for (auto geom : sceneDesc->geometries.elements) {

			if (!geom) {
				continue;
			}
			PushConstant pc{ //
				geom->transform, glm::inverseTranspose(glm::mat3(geom->transform))
			};

			for (auto& gg : geom->mesh.Lock().geometryGroups) {
				PROFILE_SCOPE(Renderer);

				auto& mat = gg.material.Lock();
				auto& arch = mat.archetype.Lock();


				if (arch.isUnlit)
					[[unlikely]] { continue; }
				auto& plLayout = *arch.gbuffer.pipelineLayout;

				cmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *arch.gbuffer.pipeline);
				cmdBuffer->pushConstants(plLayout, vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

				if (mat.hasDescriptorSet) {
					cmdBuffer->bindDescriptorSets(
						vk::PipelineBindPoint::eGraphics, plLayout, 0u, 1u, &mat.descSet, 0u, nullptr);
				}

				cmdBuffer->bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics, plLayout, 1u, 1u, &descSet, 0u, nullptr);

				auto& gpuMesh = geom->mesh.Lock();
				cmdBuffer->bindVertexBuffers(0u, { gpuMesh.combinedVertexBuffer.handle() }, { gg.vertexBufferOffset });
				cmdBuffer->bindIndexBuffer(
					gpuMesh.combinedIndexBuffer.handle(), gg.indexBufferOffset, vk::IndexType::eUint32);


				cmdBuffer->drawIndexed(gg.indexCount, 1u, 0u, 0u, 0u);
			}
		}


		for (auto geom : sceneDesc->animatedGeometries.elements) {
			if (!geom) {
				continue;
			}
			PushConstant pc{ //
				geom->transform, glm::inverseTranspose(glm::mat3(geom->transform))
			};

			for (auto& gg : geom->mesh.Lock().geometryGroups) {
				auto& mat = gg.material.Lock();
				auto& arch = mat.archetype.Lock();
				if (arch.isUnlit)
					[[unlikely]] { continue; }

				auto& plLayout = *arch.gbufferAnimated.pipelineLayout;

				cmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *arch.gbufferAnimated.pipeline);
				cmdBuffer->pushConstants(plLayout, vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

				if (mat.hasDescriptorSet) {
					cmdBuffer->bindDescriptorSets(
						vk::PipelineBindPoint::eGraphics, plLayout, 0u, 1u, &mat.descSet, 0u, nullptr);
				}

				cmdBuffer->bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics, plLayout, 1u, 1u, &descSet, 0u, nullptr);

				cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, plLayout, 2u, 1u,
					&geom->descSet[sceneDesc.frameIndex], 0u, nullptr);

				auto& gpuMesh = geom->mesh.Lock();
				cmdBuffer->bindVertexBuffers(0u, { gpuMesh.combinedVertexBuffer.handle() }, { gg.vertexBufferOffset });
				cmdBuffer->bindIndexBuffer(
					gpuMesh.combinedIndexBuffer.handle(), gg.indexBufferOffset, vk::IndexType::eUint32);

				cmdBuffer->drawIndexed(gg.indexCount, 1u, 0u, 0u, 0u);
			}
		}
	}
	cmdBuffer->end();
}

} // namespace vl
