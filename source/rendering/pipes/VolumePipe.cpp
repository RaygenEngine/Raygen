#include "VolumePipe.h"

#include "editor/Editor.h"
#include "engine/console/ConsoleVariable.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/SceneCamera.h"
#include "universe/Universe.h"
#include "universe/components/CameraComponent.h"
#include "universe/components/DirlightComponent.h"
#include "universe/components/IrragridComponent.h"
#include "universe/components/PointlightComponent.h"
#include "universe/components/QuadlightComponent.h"
#include "universe/components/ReflprobeComponent.h"
#include "universe/components/SpotlightComponent.h"

namespace {
struct PushConstant {
	glm::mat4 volumeMatVp;
	glm::vec4 color;
};

static_assert(sizeof(PushConstant) <= 128);

vk::UniquePipelineLayout MakePipelineLayout()
{
	vk::PushConstantRange pushConstantRange{};
	pushConstantRange
		.setStageFlags(vk::ShaderStageFlagBits::eVertex) //
		.setSize(sizeof(PushConstant))
		.setOffset(0u);

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.setPushConstantRanges(pushConstantRange);

	return vl::Device->createPipelineLayoutUnique(pipelineLayoutInfo);
}

template<typename PipeType>
vk::UniquePipeline MakePipeline(vk::PipelineLayout layout)
{
	GpuAsset<Shader>& gpuShader = vl::GpuAssetManager->CompileShader("engine-data/spv/unlit/volume.shader");
	gpuShader.onCompile = [&]() {
		vl::StaticPipes::Recompile<PipeType>();
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
	if constexpr (std::is_base_of_v<vl::VolumePointsPipe, PipeType>) {
		inputAssembly.setTopology(vk::PrimitiveTopology::ePointList);
	}
	if constexpr (std::is_base_of_v<vl::VolumeLinesPipe, PipeType>) {
		inputAssembly.setTopology(vk::PrimitiveTopology::eLineList);
	}
	if constexpr (std::is_base_of_v<vl::VolumeTrianglesPipe, PipeType>) {
		inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList);
	}
	inputAssembly.setPrimitiveRestartEnable(VK_FALSE);

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
		.setLayout(layout)
		.setRenderPass(vl::Layouts->unlitPassLayout.compatibleRenderPass.get())
		.setSubpass(0u)
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	return vl::Device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
}

} // namespace

namespace vl {

vk::UniquePipelineLayout VolumePointsPipe::MakePipelineLayout()
{
	return ::MakePipelineLayout();
}

vk::UniquePipeline VolumePointsPipe::MakePipeline()
{
	return ::MakePipeline<VolumePointsPipe>(layout());
}

void VolumePointsPipe::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, Entity& entity) const {}

vk::UniquePipelineLayout VolumeLinesPipe::MakePipelineLayout()
{
	return ::MakePipelineLayout();
}

vk::UniquePipeline VolumeLinesPipe::MakePipeline()
{
	return ::MakePipeline<VolumeLinesPipe>(layout());
}

void VolumeLinesPipe::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, Entity& entity) const
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline());

	if (entity.Has<CCamera>()) {
		auto cm = entity.Get<CCamera>();


		auto volumeTransform
			= math::transformMat(glm::vec3{ 0.3 }, entity->world().orientation, entity->world().position);

		PushConstant pc{ sceneDesc.viewer.ubo.viewProj * glm::inverse(cm.proj * cm.view),
			glm::vec4(1.f, 1.f, 1.f, 1.f) };

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		cmdBuffer.setDepthTestEnableEXT(VK_TRUE);
		cmdBuffer.setPrimitiveTopologyEXT(vk::PrimitiveTopology::eLineList);

		// ndc cube transformed by viewProjInv = frustum
		rvk::bindCubeLines(cmdBuffer);
		rvk::drawCubeLines(cmdBuffer);
	}

	if (entity.Has<CSpotlight>()) {
		auto sl = entity.Get<CSpotlight>();

		auto volumeTransform
			= math::transformMat(glm::vec3{ 1 }, entity->world().orientation, entity->world().position);

		PushConstant pc{ sceneDesc.viewer.ubo.viewProj * glm::inverse(sl.proj * sl.view),
			glm::vec4(1.f, 1.f, 1.f, 1.f) };

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		cmdBuffer.setDepthTestEnableEXT(VK_TRUE);
		cmdBuffer.setPrimitiveTopologyEXT(vk::PrimitiveTopology::eLineList);

		// ndc cube transformed by viewProjInv = frustum
		rvk::bindCubeLines(cmdBuffer);
		rvk::drawCubeLines(cmdBuffer); // TODO: cone from frustum and spot effect
	}

	if (entity.Has<CDirlight>()) {
		auto dl = entity.Get<CDirlight>();

		auto volumeTransform
			= math::transformMat(glm::vec3{ 1 }, entity->world().orientation, entity->world().position);

		PushConstant pc{ sceneDesc.viewer.ubo.viewProj * glm::inverse(dl.proj * dl.view),
			glm::vec4(1.f, 1.f, 1.f, 1.f) };

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		cmdBuffer.setDepthTestEnableEXT(VK_TRUE);
		cmdBuffer.setPrimitiveTopologyEXT(vk::PrimitiveTopology::eLineList);

		rvk::bindCubeLines(cmdBuffer);
		rvk::drawCubeLines(cmdBuffer);
	}

	if (entity.Has<CIrragrid>()) {
		auto ig = entity.Get<CIrragrid>();

		math::AABB igAabb{
			entity->world().position,
			entity->world().position + glm::vec3(ig.width - 1, ig.height - 1, ig.depth - 1) * ig.distToAdjacent,
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


	// TODO: draw SkinnedMesh animated skeleton
}

vk::UniquePipelineLayout VolumeTrianglesPipe::MakePipelineLayout()
{
	return ::MakePipelineLayout();
}

vk::UniquePipeline VolumeTrianglesPipe::MakePipeline()
{
	return ::MakePipeline<VolumeTrianglesPipe>(layout());
}

void VolumeTrianglesPipe::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, Entity& entity) const
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline());

	if (entity.Has<CPointlight>()) {
		auto pl = entity.Get<CPointlight>();

		auto volumeTransform = math::transformMat(
			glm::vec3{ pl.CalculateEffectiveRadius() }, entity->world().orientation, entity->world().position);

		PushConstant pc{ sceneDesc.viewer.ubo.viewProj * volumeTransform, glm::vec4(1.f, 1.f, 1.f, 1.f) };

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		cmdBuffer.setDepthTestEnableEXT(VK_TRUE);
		cmdBuffer.setPrimitiveTopologyEXT(vk::PrimitiveTopology::eTriangleList);

		rvk::bindSphere18x9(cmdBuffer);
		rvk::drawSphere18x9(cmdBuffer);
	}

	if (entity.Has<CReflprobe>()) {
		auto rp = entity.Get<CReflprobe>();

		auto volumeTransform
			= math::transformMat(glm::vec3{ rp.radius }, entity->world().orientation, entity->world().position);

		PushConstant pc{ sceneDesc.viewer.ubo.viewProj * volumeTransform, glm::vec4(1.f, 1.f, 1.f, 1.f) };

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		cmdBuffer.setDepthTestEnableEXT(VK_TRUE);
		cmdBuffer.setPrimitiveTopologyEXT(vk::PrimitiveTopology::eTriangleList);

		rvk::bindSphere18x9(cmdBuffer);
		rvk::drawSphere18x9(cmdBuffer);
	}

	// TODO: this should change later to light volume
	if (entity.Has<CQuadlight>()) {
		auto ql = entity.Get<CQuadlight>();

		auto volumeTransform = math::transformMat(
			glm::vec3{ ql.width, ql.height, 1.f }, entity->world().orientation, entity->world().position);

		PushConstant pc{ sceneDesc.viewer.ubo.viewProj * volumeTransform, glm::vec4(1.f, 1.f, 1.f, 1.f) };

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		cmdBuffer.setDepthTestEnableEXT(VK_TRUE);
		cmdBuffer.setPrimitiveTopologyEXT(vk::PrimitiveTopology::eTriangleStrip);

		rvk::bindUnitRectTriangleStrip(cmdBuffer);
		rvk::drawUnitRectTriangleStrip(cmdBuffer);
	}
}

} // namespace vl
