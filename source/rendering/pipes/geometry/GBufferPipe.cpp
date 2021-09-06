#include "GbufferPipe.h"

#include "assets/AssetRegistry.h"
#include "assets/shared/GeometryShared.h"
#include "rendering/assets/GpuMaterialArchetype.h"
#include "rendering/assets/GpuMaterialInstance.h"
#include "rendering/assets/GpuSkinnedMesh.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/SceneGeometry.h"

#include <glm/gtc/matrix_inverse.hpp>

namespace {
struct PushConstant {
	glm::mat4 modelMat;
	glm::mat4 normalMat;
	glm::mat4 mvpPrev;
	float drawIndex;
};

// TODO:
// static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
size_t GbufferPipe::GetPushConstantSize()
{
	return sizeof(PushConstant);
}

namespace {
	vk::UniquePipeline CreatePipelineFromVtxInfo(vk::PipelineLayout pipelineLayout,
		std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages,
		vk::PipelineVertexInputStateCreateInfo vertexInputState)
	{
		std::array dynamicStates{
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor,
			vk::DynamicState::eCullModeEXT,
		};
		vk::PipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.setDynamicStates(dynamicStates);

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

		auto colorAttCount = PassLayouts->gBufferColorAttachments.size();

		std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachment{};
		for (uint32 i = 0u; i < colorAttCount; ++i) {
			colorBlendAttachment.emplace_back();
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


		return rvk::makeGraphicsPipeline(shaderStages, &vertexInputState, nullptr, nullptr, nullptr, &rasterizer,
			nullptr, &depthStencil, &colorBlending, &dynamicState, pipelineLayout,
			PassLayouts->main.compatibleRenderPass.get(), 0u);
	}
} // namespace

vk::UniquePipeline GbufferPipe::CreatePipeline(
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

vk::UniquePipeline GbufferPipe::CreateAnimPipeline(
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
		.setVertexBindingDescriptions(bindingDescription) //
		.setVertexAttributeDescriptions(attributeDescriptions);

	return CreatePipelineFromVtxInfo(pipelineLayout, shaderStages, vertexInputInfo);
}

void GbufferPipe::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	COMMAND_SCOPE(cmdBuffer, "GbufferPipe::RecordCmd");

	for (auto& geom : sceneDesc->Get<SceneGeometry>()) {

		COMMAND_SCOPE(cmdBuffer, "Model Draw: " + AssetRegistry::GetPodUri(BasePodHandle{ geom->mesh.uid }));

		PushConstant pc{
			.modelMat = geom->transform,
			.normalMat = glm::inverseTranspose(glm::mat3(geom->transform)),
			.mvpPrev = sceneDesc.viewer.prevViewProj * geom->prevTransform,
			.drawIndex = 0.f,
		};

		for (auto& gg : geom->mesh.Lock().geometryGroups) {
			COMMAND_SCOPE(cmdBuffer, "Geometry Group Draw");

			auto& mat = gg.material.Lock();
			auto& arch = mat.archetype.Lock();


			mat.doubleSided ? cmdBuffer.setCullModeEXT(vk::CullModeFlagBits::eNone)
							: cmdBuffer.setCullModeEXT(vk::CullModeFlagBits::eBack);

			if (arch.isUnlit) [[unlikely]] {
				continue;
			}
			auto& plLayout = *arch.gbuffer.pipelineLayout;


			pc.drawIndex = float(gg.material.uid);

			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *arch.gbuffer.pipeline);
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


	for (auto geom : sceneDesc->Get<SceneAnimatedGeometry>()) {

		COMMAND_SCOPE(cmdBuffer, "Skinned Model Draw: " + AssetRegistry::GetPodUri(BasePodHandle{ geom->mesh.uid }));

		PushConstant pc{
			.modelMat = geom->transform,
			.normalMat = glm::inverseTranspose(glm::mat3(geom->transform)),
		};

		for (auto& gg : geom->mesh.Lock().geometryGroups) {

			COMMAND_SCOPE(cmdBuffer, "Skinned Geometry Group Draw");

			auto& mat = gg.material.Lock();
			auto& arch = mat.archetype.Lock();
			if (arch.isUnlit) [[unlikely]] {
				continue;
			}

			auto& plLayout = *arch.gbufferAnimated.pipelineLayout;

			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *arch.gbufferAnimated.pipeline);
			cmdBuffer.pushConstants(plLayout, vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

			cmdBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics, plLayout, 0u, { sceneDesc.globalDesc }, nullptr);


			if (mat.hasDescriptorSet) {
				cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, plLayout, 1u, { mat.descSet }, nullptr);
			}

			cmdBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics, plLayout, 2u, { geom->descSet[sceneDesc.frameIndex] }, nullptr);

			auto& gpuMesh = geom->mesh.Lock();
			cmdBuffer.bindVertexBuffers(0u, { gpuMesh.combinedVertexBuffer.handle() }, { gg.vertexBufferOffset });
			cmdBuffer.bindIndexBuffer(
				gpuMesh.combinedIndexBuffer.handle(), gg.indexBufferOffset, vk::IndexType::eUint32);

			cmdBuffer.drawIndexed(gg.indexCount, 1u, 0u, 0u, 0u);
		}
	}
}

} // namespace vl
