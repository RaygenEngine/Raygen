#include "UnlitVolumePass.h"

#include "editor/Editor.h"
#include "engine/console/ConsoleVariable.h"
#include "rendering/StaticPipes.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/util/DrawShapes.h"
#include "universe/Universe.h"
#include "universe/components/CameraComponent.h"
#include "universe/components/DirlightComponent.h"
#include "universe/components/IrragridComponent.h"
#include "universe/components/PointlightComponent.h"
#include "universe/components/ReflprobeComponent.h"
#include "universe/components/SkinnedMeshComponent.h"
#include "universe/components/SpotlightComponent.h"


namespace {
struct PushConstant {
	glm::mat4 volumeMatVp;
	glm::vec4 color;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace


namespace vl {

vk::UniquePipelineLayout UnlitVolumePass::MakePipelineLayout()
{
	vk::PushConstantRange pushConstantRange{};
	pushConstantRange
		.setStageFlags(vk::ShaderStageFlagBits::eVertex) //
		.setSize(sizeof(PushConstant))
		.setOffset(0u);

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.setPushConstantRanges(pushConstantRange);

	return Device->createPipelineLayoutUnique(pipelineLayoutInfo);
}

vk::UniquePipeline UnlitVolumePass::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/unlit/volume.shader");
	gpuShader.onCompile = [&]() {
		StaticPipes::Recompile<UnlitVolumePass>();
	};

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

	// Dynamic vieport
	std::array dynamicStates{
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,
		vk::DynamicState::ePrimitiveTopologyEXT,
		vk::DynamicState::eDepthTestEnableEXT,
	};

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
		.setPolygonMode(vk::PolygonMode::eLine)
		.setLineWidth(1.f)
		.setCullMode(vk::CullModeFlagBits::eNone)
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

	// depth and stencil state
	vk::PipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil
		.setDepthTestEnable(VK_TRUE) //
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
		.setStages(gpuShader.shaderStages) //
		.setPVertexInputState(&vertexInputInfo)
		.setPInputAssemblyState(&inputAssembly)
		.setPViewportState(&viewportState)
		.setPRasterizationState(&rasterizer)
		.setPMultisampleState(&multisampling)
		.setPDepthStencilState(&depthStencil)
		.setPColorBlendState(&colorBlending)
		.setPDynamicState(&dynamicStateInfo)
		.setLayout(layout())
		.setRenderPass(Layouts->ptPassLayout.compatibleRenderPass.get())
		.setSubpass(0u)
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	return Device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
}

void UnlitVolumePass::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline());

	auto selEnt = Editor::GetSelection();

	if (selEnt && selEnt.Has<CPointlight>()) {
		auto pl = selEnt.Get<CPointlight>();

		auto volumeTransform = math::transformMat(
			glm::vec3{ pl.CalculateEffectiveRadius() }, selEnt->world().orientation, selEnt->world().position);

		PushConstant pc{ sceneDesc.viewer.ubo.viewProj * volumeTransform, glm::vec4(1.f, 1.f, 1.f, 1.f) };

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		cmdBuffer.setDepthTestEnableEXT(VK_TRUE);
		cmdBuffer.setPrimitiveTopologyEXT(vk::PrimitiveTopology::eTriangleList);

		rvk::bindSphere18x9(cmdBuffer);
		rvk::drawSphere18x9(cmdBuffer);
	}

	if (selEnt && selEnt.Has<CReflprobe>()) {
		auto rp = selEnt.Get<CReflprobe>();

		auto volumeTransform
			= math::transformMat(glm::vec3{ rp.radius }, selEnt->world().orientation, selEnt->world().position);

		PushConstant pc{ sceneDesc.viewer.ubo.viewProj * volumeTransform, glm::vec4(1.f, 1.f, 1.f, 1.f) };

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		cmdBuffer.setDepthTestEnableEXT(VK_TRUE);
		cmdBuffer.setPrimitiveTopologyEXT(vk::PrimitiveTopology::eTriangleList);

		rvk::bindSphere18x9(cmdBuffer);
		rvk::drawSphere18x9(cmdBuffer);
	}

	if (selEnt && selEnt.Has<CCamera>()) {
		auto cm = selEnt.Get<CCamera>();


		auto volumeTransform
			= math::transformMat(glm::vec3{ 0.3 }, selEnt->world().orientation, selEnt->world().position);

		PushConstant pc{ sceneDesc.viewer.ubo.viewProj * glm::inverse(cm.proj * cm.view),
			glm::vec4(1.f, 1.f, 1.f, 1.f) };

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		cmdBuffer.setDepthTestEnableEXT(VK_TRUE);
		cmdBuffer.setPrimitiveTopologyEXT(vk::PrimitiveTopology::eLineList);

		// ndc cube transformed by viewProjInv = frustum
		rvk::bindCubeLines(cmdBuffer);
		rvk::drawCubeLines(cmdBuffer);
	}

	if (selEnt && selEnt.Has<CSpotlight>()) {
		auto sl = selEnt.Get<CSpotlight>();

		auto volumeTransform
			= math::transformMat(glm::vec3{ 1 }, selEnt->world().orientation, selEnt->world().position);

		PushConstant pc{ sceneDesc.viewer.ubo.viewProj * glm::inverse(sl.proj * sl.view),
			glm::vec4(1.f, 1.f, 1.f, 1.f) };

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		cmdBuffer.setDepthTestEnableEXT(VK_TRUE);
		cmdBuffer.setPrimitiveTopologyEXT(vk::PrimitiveTopology::eLineList);

		// ndc cube transformed by viewProjInv = frustum
		rvk::bindCubeLines(cmdBuffer);
		rvk::drawCubeLines(cmdBuffer); // TODO: cone from frustum and spot effect
	}

	if (selEnt && selEnt.Has<CDirlight>()) {
		auto dl = selEnt.Get<CDirlight>();

		auto volumeTransform
			= math::transformMat(glm::vec3{ 1 }, selEnt->world().orientation, selEnt->world().position);

		PushConstant pc{ sceneDesc.viewer.ubo.viewProj * glm::inverse(dl.proj * dl.view),
			glm::vec4(1.f, 1.f, 1.f, 1.f) };

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		cmdBuffer.setDepthTestEnableEXT(VK_TRUE);
		cmdBuffer.setPrimitiveTopologyEXT(vk::PrimitiveTopology::eLineList);

		rvk::bindCubeLines(cmdBuffer);
		rvk::drawCubeLines(cmdBuffer);
	}

	if (selEnt && selEnt.Has<CIrragrid>()) {
		auto ig = selEnt.Get<CIrragrid>();

		math::AABB igAabb{
			selEnt->world().position,
			selEnt->world().position + glm::vec3(ig.width - 1, ig.height - 1, ig.depth - 1) * ig.distToAdjacent,
		};

		auto volumeTransform
			= math::transformMat(glm::vec3{ igAabb.GetExtend() }, glm::identity<glm::quat>(), igAabb.GetCenter());

		PushConstant pc{ sceneDesc.viewer.ubo.viewProj * volumeTransform, glm::vec4(1.f, 1.f, 0.3f, 1.f) };

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		cmdBuffer.setDepthTestEnableEXT(VK_FALSE);
		cmdBuffer.setPrimitiveTopologyEXT(vk::PrimitiveTopology::eLineList);

		rvk::bindCubeLines(cmdBuffer);
		rvk::drawCubeLines(cmdBuffer);
	}

	// TODO: draw SkinnedMesh skeleton

	// TODO:
	static ConsoleVariable<bool> cons_drawLeaves{ "s.bvh.Children", false };
	if (!cons_drawLeaves) {
		return;
	}

	if (Universe::MainWorld->physics.tree) {
		cmdBuffer.setDepthTestEnableEXT(VK_FALSE);
		cmdBuffer.setPrimitiveTopologyEXT(vk::PrimitiveTopology::eLineList);

		rvk::bindCubeLines(cmdBuffer);

		for (auto& node : Universe::MainWorld->physics.tree->nodes) {

			auto volumeTransform = math::transformMat(
				glm::vec3{ node.aabb.GetExtend() }, glm::identity<glm::quat>(), node.aabb.GetCenter());

			PushConstant pc{ sceneDesc.viewer.ubo.viewProj * volumeTransform, glm::vec4(1.f, 1.f, 1.f, 1.f) };

			if (node.isLeaf) {
				pc.color.y = 0.f;

				if (Editor::GetSelection() != node.data) {
					pc.color.z = 0.f;
				}
			}

			static ConsoleVariable<bool> cons_draw{ "s.bvh.Debug", false };

			if (cons_draw || node.isLeaf) {
				cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);
				rvk::drawCubeLines(cmdBuffer);
			}
		}
	}
}

} // namespace vl
