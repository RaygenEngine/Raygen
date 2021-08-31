#include "UnlitPipe.h"

#include "assets/shared/GeometryShared.h"
#include "rendering/assets/GpuMaterialArchetype.h"
#include "rendering/assets/GpuMaterialInstance.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneGeometry.h"

#include <glm/gtc/matrix_inverse.hpp>

namespace {
struct PushConstant {
	glm::mat4 modelMat;
	glm::mat4 normalMat;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {

size_t UnlitPipe::GetPushConstantSize()
{
	return sizeof(PushConstant);
}

namespace {
	vk::UniquePipeline CreatePipelineFromVtxInfo(vk::PipelineLayout pipelineLayout,
		std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages,
		vk::PipelineVertexInputStateCreateInfo vertexInputState)
	{
		static ConsoleVariable<vk::PolygonMode> cons_fillMode{ "r.unlit.fillMode", vk::PolygonMode::eFill,
			"Fill mode for unlit custom shaders. Recompile the archetype to apply." };

		// Dynamic vieport
		std::array dynamicStates{
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor,
		};
		vk::PipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.setDynamicStates(dynamicStates);

		vk::PipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer
			.setDepthClampEnable(VK_FALSE) //
			.setRasterizerDiscardEnable(VK_FALSE)
			.setPolygonMode(cons_fillMode)
			.setLineWidth(1.f)
			.setCullMode(vk::CullModeFlagBits::eBack)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setDepthBiasEnable(VK_FALSE)
			.setDepthBiasConstantFactor(0.f)
			.setDepthBiasClamp(0.f)
			.setDepthBiasSlopeFactor(0.f);

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

		return rvk::makeGraphicsPipeline(shaderStages, &vertexInputState, nullptr, nullptr, nullptr, &rasterizer,
			nullptr, &depthStencil, nullptr, &dynamicState, pipelineLayout,
			PassLayouts->unlit.compatibleRenderPass.get(), 1u);
	}
} // namespace

vk::UniquePipeline UnlitPipe::CreatePipeline(
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


void UnlitPipe::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	COMMAND_SCOPE(cmdBuffer, "UnlitPipe::RecordCmd");

	for (auto geom : sceneDesc->Get<SceneGeometry>()) {

		COMMAND_SCOPE(cmdBuffer, "Model Draw");

		PushConstant pc{ //
			geom->transform, glm::inverseTranspose(glm::mat3(geom->transform))
		};

		for (auto& gg : geom->mesh.Lock().geometryGroups) {

			COMMAND_SCOPE(cmdBuffer, "Geometry Group Draw");

			auto& mat = gg.material.Lock();
			auto& arch = mat.archetype.Lock();
			if (!arch.isUnlit) [[likely]] {
				continue;
			}

			auto& plLayout = *arch.unlit.pipelineLayout;

			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *arch.unlit.pipeline);
			cmdBuffer.pushConstants(plLayout, vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

			cmdBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics, plLayout, 0u, { sceneDesc.globalDesc }, nullptr);

			if (mat.hasDescriptorSet) {
				cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, plLayout, 1u, { mat.descSet }, nullptr);
			}

			auto& gpuMesh = geom->mesh.Lock();
			cmdBuffer.bindVertexBuffers(0u, { gpuMesh.combinedVertexBuffer.handle() }, { gg.vertexBufferOffset });
			cmdBuffer.bindIndexBuffer(
				gpuMesh.combinedIndexBuffer.handle(), gg.indexBufferOffset, vk::IndexType::eUint32);


			cmdBuffer.drawIndexed(gg.indexCount, 1u, 0u, 0u, 0u);
		}
	}

	// CHECK: Unlit Animations
}


} // namespace vl
