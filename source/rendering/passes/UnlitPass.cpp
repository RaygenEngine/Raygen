#include "pch.h"
#include "UnlitPass.h"

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
#include "engine/console/ConsoleVariable.h"
#include "rendering/scene/SceneGeometry.h"
#include "rendering/scene/SceneCamera.h"
#include "assets/shared/GeometryShared.h"

#include <glm/gtc/matrix_inverse.hpp>

namespace {
struct PushConstant {
	glm::mat4 modelMat;
	glm::mat4 normalMat;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {

size_t UnlitPass::GetPushConstantSize()
{
	return sizeof(PushConstant);
}

namespace {
	vk::UniquePipeline CreatePipelineFromVtxInfo(vk::PipelineLayout pipelineLayout,
		std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages,
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo)
	{
		static ConsoleVariable<vk::PolygonMode> unlitFillModeConsole{ "r.unlitFillMode", vk::PolygonMode::eFill,
			"Fill mode for unlit custom shaders. Recompile the archetype to apply." };

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly
			.setTopology(vk::PrimitiveTopology::eTriangleList) //
			.setPrimitiveRestartEnable(VK_FALSE);

		// Dynamic vieport
		std::array dynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
		vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
		dynamicStateInfo.setDynamicStates(dynamicStates);

		// those are dynamic so they will be updated when needed
		vk::Viewport viewport{};
		vk::Rect2D scissor{};
		vk::PipelineViewportStateCreateInfo viewportState{};
		viewportState
			.setViewports(viewport) //
			.setScissors(scissor);

		vk::PipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer
			.setDepthClampEnable(VK_FALSE) //
			.setRasterizerDiscardEnable(VK_FALSE)
			.setPolygonMode(unlitFillModeConsole)
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

		std::array<vk::PipelineColorBlendAttachmentState, 1> colorBlendAttachment{};
		for (uint32 i = 0u; i < 1; ++i) {
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
			.setAttachments(colorBlendAttachment)
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
			.setStages(shaderStages) //
			.setPVertexInputState(&vertexInputInfo)
			.setPInputAssemblyState(&inputAssembly)
			.setPViewportState(&viewportState)
			.setPRasterizationState(&rasterizer)
			.setPMultisampleState(&multisampling)
			.setPDepthStencilState(&depthStencil)
			.setPColorBlendState(&colorBlending)
			.setPDynamicState(&dynamicStateInfo)
			.setLayout(pipelineLayout)
			.setRenderPass(Layouts->ptPassLayout.compatibleRenderPass.get())
			.setSubpass(1u)
			.setBasePipelineHandle({})
			.setBasePipelineIndex(-1);

		return Device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
	}
} // namespace

vk::UniquePipeline UnlitPass::CreatePipeline(
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
		.setVertexBindingDescriptions(bindingDescription) //
		.setVertexAttributeDescriptions(attributeDescriptions);

	return CreatePipelineFromVtxInfo(pipelineLayout, shaderStages, vertexInputInfo);
}


void UnlitPass::RecordCmd(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	PROFILE_SCOPE(Renderer);

	auto camera = sceneDesc.viewer;
	if (!camera) {
		cmdBuffer->endRenderPass();
		return;
	}

	for (auto geom : sceneDesc->geometries.elements) {
		if (!geom) {
			continue;
		}
		PushConstant pc{ //
			geom->transform, glm::inverseTranspose(glm::mat3(geom->transform))
		};

		for (auto& gg : geom->mesh.Lock().geometryGroups) {
			auto& mat = gg.material.Lock();
			auto& arch = mat.archetype.Lock();
			if (!arch.isUnlit)
				[[likely]] { continue; }

			auto& plLayout = *arch.unlit.pipelineLayout;

			cmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *arch.unlit.pipeline);
			cmdBuffer->pushConstants(plLayout, vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

			if (mat.hasDescriptorSet) {
				cmdBuffer->bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics, plLayout, 0u, 1u, &mat.descSet, 0u, nullptr);
			}

			cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, plLayout, 1u, 1u,
				&camera->descSet[sceneDesc.frameIndex], 0u, nullptr);

			auto& gpuMesh = geom->mesh.Lock();
			cmdBuffer->bindVertexBuffers(0u, { gpuMesh.combinedVertexBuffer.handle() }, { gg.vertexBufferOffset });
			cmdBuffer->bindIndexBuffer(
				gpuMesh.combinedIndexBuffer.handle(), gg.indexBufferOffset, vk::IndexType::eUint32);


			cmdBuffer->drawIndexed(gg.indexCount, 1u, 0u, 0u, 0u);
		}
		// TODO: Unlit Animations
	}
}

} // namespace vl
