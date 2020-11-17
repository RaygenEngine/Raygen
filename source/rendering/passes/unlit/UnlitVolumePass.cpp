#include "UnlitVolumePass.h"

#include "editor/Editor.h"
#include "core/math-ext/BVH.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/passes/lightblend/PointlightBlend.h"
#include "rendering/Renderer.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/ScenePointlight.h"
#include "rendering/StaticPipes.h"
#include "universe/components/PointlightComponent.h"
#include "universe/Universe.h"
#include "universe/World.h"
#include "engine/console/ConsoleVariable.h"
#include "engine/Input.h"
#include "editor/windows/general/EdOutlinerWindow.h"

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

std::unique_ptr<math::BVH<Entity>> bvh;

// clang-format off
 ConsoleFunction<> buildBvh{ "s.bvh", []() {
	using namespace math;
	bvh = std::make_unique<math::BVH<Entity>>();

	glm::vec3 h = {1, 1, 1};
	AABB cube = {-h, h};

	auto world = Universe::MainWorld;
	for (auto [entity, bc] : world->GetView<BasicComponent>().each()) {
		bvh->InsertLeaf(bc.self, cube.Transform(bc.world().transform));
	}
} };
// clang-format on
void UnlitVolumePass::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline());

	auto selEnt = Editor::GetSelection();

	if (selEnt && selEnt.Has<CPointlight>()) {
		auto pl = selEnt.Get<CPointlight>();
		// WIP:
		const auto& relPipe = StaticPipes::Get<PointlightBlend>();

		// bind unit sphere once
		cmdBuffer.bindVertexBuffers(0u, relPipe.m_sphereVertexBuffer.handle(), vk::DeviceSize(0));
		cmdBuffer.bindIndexBuffer(
			relPipe.m_sphereIndexBuffer.buffer.handle(), vk::DeviceSize(0), vk::IndexType::eUint32);

		auto volumeTransform = math::transformMat(
			glm::vec3{ pl.CalculateEffectiveRadius() }, selEnt->world().orientation, selEnt->world().position);

		PushConstant pc{ sceneDesc.viewer.ubo.viewProj * volumeTransform, glm::vec4(1.f, 1.f, 1.f, 1.f) };

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		cmdBuffer.drawIndexed(relPipe.m_sphereIndexBuffer.count, 1u, 0u, 0u, 0u);
	}

	if (bvh) {
		for (auto& node : bvh->nodes) {
			const auto& cubeVtxBuf = StaticPipes::Get<IrradianceMapCalculation>().m_cubeVertexBuffer;


			auto volumeTransform = math::transformMat(
				glm::vec3{ node.aabb.GetExtend() }, glm::identity<glm::quat>(), node.aabb.GetCenter());

			PushConstant pc{ sceneDesc.viewer.ubo.viewProj * volumeTransform, glm::vec4(1.f, 1.f, 1.f, 1.f) };

			if (node.isLeaf) {
				pc.color.y = 0.f;

				if (Editor::GetSelection() != node.data) {
					pc.color.z = 0.f;
				}
			}

			cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);
			cmdBuffer.bindVertexBuffers(0u, { cubeVtxBuf.handle() }, { vk::DeviceSize(0) });
			cmdBuffer.draw(static_cast<uint32>(108 / 3), 1u, 0u, 0u);
		}
	}

	using namespace math;
	bvh = std::make_unique<math::BVH<Entity>>();

	glm::vec3 h = { 1, 1, 1 };
	AABB cube = { -h, h };

	bool debug{ false };
	if (Input.IsJustPressed(Key::N)) {
		debug = true;
		LOG_REPORT("");
	}

	auto world = Universe::MainWorld;
	for (auto [entity, bc] : world->GetView<BasicComponent>().each()) {
		CLOG_REPORT(debug, "Inserting: {}", bc.name);
		bvh->InsertLeaf(bc.self, cube.Transform(bc.world().transform));
	}


	if (Input.IsJustPressed(Key::Space)) {
		auto view = sceneDesc.viewer.ubo.view;
		glm::vec4 cameraFwd{ -view[0][2], -view[1][2], -view[2][2], 0.f };

		auto results = bvh->RayCastDirection(sceneDesc.viewer.ubo.position, cameraFwd, 1000.f);
		if (!results.distanceSqToHitObject.empty()) {
			auto entId = results.distanceSqToHitObject.begin()->second;

			ed::OutlinerWindow::selected = entId;
		}
	}
}
} // namespace vl
