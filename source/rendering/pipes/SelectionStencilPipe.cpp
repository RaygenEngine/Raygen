#include "SelectionStencilPipe.h"

#include "editor/Editor.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/SceneGeometry.h"
#include "universe/components/StaticMeshComponent.h"
#include "universe/Universe.h"


namespace {
struct PushConstant {
	glm::mat4 mvp;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace


namespace vl {

vk::UniquePipelineLayout SelectionStencilPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayout<PushConstant>({}, vk::ShaderStageFlagBits::eVertex);
}

vk::UniquePipeline SelectionStencilPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/unlit/selection.shader");
	gpuShader.onCompile = [&]() {
		StaticPipes::Recompile<SelectionStencilPipe>();
	};

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


	vk::PipelineVertexInputStateCreateInfo vertexInputState{};
	vertexInputState
		.setVertexBindingDescriptions(bindingDescription) //
		.setVertexAttributeDescriptions(attributeDescriptions);

	std::array dynamicStates{
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,
		vk::DynamicState::eDepthTestEnableEXT,
		vk::DynamicState::eDepthWriteEnableEXT,
		vk::DynamicState::eStencilOpEXT,
	};

	vk::PipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.setDynamicStates(dynamicStates);

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
		nullptr, nullptr, nullptr, &dynamicState, layout(), PassLayouts->unlit.compatibleRenderPass.get(), 0u);
}

void SelectionStencilPipe::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline());

	auto selEnt = Editor::GetSelection();

	if (selEnt && selEnt.Has<CStaticMesh>()) {
		auto pl = selEnt.Get<CStaticMesh>();

		auto geom = sceneDesc->GetElement<SceneGeometry>(pl.sceneUid);

		PushConstant pc{
			sceneDesc.viewer.ubo.viewProj * geom->transform,
		};

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		for (auto& gg : geom->mesh.Lock().geometryGroups) {

			auto& gpuMesh = geom->mesh.Lock();
			cmdBuffer.bindVertexBuffers(0u, { gpuMesh.combinedVertexBuffer.handle() }, { gg.vertexBufferOffset });
			cmdBuffer.bindIndexBuffer(
				gpuMesh.combinedIndexBuffer.handle(), gg.indexBufferOffset, vk::IndexType::eUint32);

			cmdBuffer.setStencilOpEXT(vk::StencilFaceFlagBits::eFrontAndBack, vk::StencilOp::eReplace,
				vk::StencilOp::eReplace, vk::StencilOp::eReplace, vk::CompareOp::eAlways);

			cmdBuffer.setStencilReference(vk::StencilFaceFlagBits::eFrontAndBack, 1);
			cmdBuffer.setStencilCompareMask(
				vk::StencilFaceFlagBits::eFrontAndBack, 0xFF); // all fragments should pass the stencil test
			cmdBuffer.setStencilWriteMask(
				vk::StencilFaceFlagBits::eFrontAndBack, 0xFF); // enable writing to the stencil buffer

			cmdBuffer.setDepthTestEnableEXT(VK_TRUE);
			cmdBuffer.setDepthWriteEnableEXT(VK_TRUE);

			cmdBuffer.drawIndexed(gg.indexCount, 1u, 0u, 0u, 0u);
		}

		pc = {
			sceneDesc.viewer.ubo.viewProj * geom->transform * glm::scale(glm::vec3(1.05f)),
		};

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		for (auto& gg : geom->mesh.Lock().geometryGroups) {
			auto& gpuMesh = geom->mesh.Lock();
			cmdBuffer.bindVertexBuffers(0u, { gpuMesh.combinedVertexBuffer.handle() }, { gg.vertexBufferOffset });
			cmdBuffer.bindIndexBuffer(
				gpuMesh.combinedIndexBuffer.handle(), gg.indexBufferOffset, vk::IndexType::eUint32);

			cmdBuffer.setStencilOpEXT(vk::StencilFaceFlagBits::eFrontAndBack, vk::StencilOp::eKeep,
				vk::StencilOp::eReplace, vk::StencilOp::eKeep, vk::CompareOp::eNotEqual);

			cmdBuffer.setStencilWriteMask(
				vk::StencilFaceFlagBits::eFrontAndBack, 0x00); // disable writing to the stencil buffer

			cmdBuffer.setDepthTestEnableEXT(VK_FALSE);
			cmdBuffer.setDepthWriteEnableEXT(VK_FALSE);

			cmdBuffer.drawIndexed(gg.indexCount, 1u, 0u, 0u, 0u);
		}
	}
}

} // namespace vl
