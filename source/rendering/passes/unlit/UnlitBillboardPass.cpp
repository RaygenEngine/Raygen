#include "UnlitBillboardPass.h"

#include "rendering/Renderer.h"
#include "rendering/StaticPipes.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/SceneIrragrid.h"
#include "universe/Universe.h"
#include "universe/components/IrragridComponent.h"
#include "universe/components/PointlightComponent.h"
#include "universe/components/ReflProbeComponent.h"

namespace {
struct PushConstant {
	glm::mat4 viewProj;
	glm::vec4 position;
	glm::vec4 cameraRight;
	glm::vec4 cameraUp;
	float scale;
};

static_assert(sizeof(PushConstant) <= 128);

// The VBO containing the 4 vertices of the particles.
} // namespace

namespace vl {

vk::UniquePipelineLayout UnlitBillboardPass::MakePipelineLayout()
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

vk::UniquePipeline UnlitBillboardPass::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/unlit/billboard.shader");
	gpuShader.onCompile = [&]() {
		StaticPipes::Recompile<UnlitBillboardPass>();
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
		.setTopology(vk::PrimitiveTopology::eTriangleStrip) //
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
		.setPolygonMode(vk::PolygonMode::eFill)
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
		.setDstColorBlendFactor(vk::BlendFactor::eOne)
		.setColorBlendOp(vk::BlendOp::eAdd)
		.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
		.setDstAlphaBlendFactor(vk::BlendFactor::eOne)
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

void UnlitBillboardPass::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline());

	cmdBuffer.bindVertexBuffers(0u, m_rectangleVertexBuffer.handle(), vk::DeviceSize(0));

	auto view = sceneDesc.viewer.ubo.view;
	glm::vec4 cameraRight{ view[0][0], view[1][0], view[2][0], 0.f };
	glm::vec4 cameraUp{ view[0][1], view[1][1], view[2][1], 0.f };

	for (auto& [ent, rp, bc] : Universe::MainWorld->GetView<CReflprobe, BasicComponent>().each()) {


		PushConstant pc{
			sceneDesc.viewer.ubo.viewProj,
			glm::vec4(bc.world().position, 1.f),
			cameraRight,
			cameraUp,
			0.2f,
		};

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		// draw rectangle
		cmdBuffer.draw(4u, 1u, 0u, 0u);
	}

	for (auto& [ent, ig, bc] : Universe::MainWorld->GetView<CIrragrid, BasicComponent>().each()) {

		if (ig.hideBillboards) {
			continue;
		}

		for (int32 x = 0; x < ig.width; ++x) {
			for (int32 y = 0; y < ig.height; ++y) {
				for (int32 z = 0; z < ig.depth; ++z) {


					auto pos = bc.world().position + glm::vec3(x, y, z) * ig.distToAdjacent;

					PushConstant pc{
						sceneDesc.viewer.ubo.viewProj,
						glm::vec4(pos, 1.f),
						cameraRight,
						cameraUp,
						0.2f,
					};

					cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

					// draw rectangle
					cmdBuffer.draw(4u, 1u, 0u, 0u);
				}
			}
		}
	}

	for (auto& [ent, rp, bc] : Universe::MainWorld->GetView<CPointlight, BasicComponent>().each()) {


		PushConstant pc{
			sceneDesc.viewer.ubo.viewProj,
			glm::vec4(bc.world().position, 1.f),
			cameraRight,
			cameraUp,
			0.4f,
		};

		cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

		// draw rectangle
		cmdBuffer.draw(4u, 1u, 0u, 0u);
	}
}

UnlitBillboardPass::UnlitBillboardPass()
{
	// TODO: std gpu asset
	// clang-format off
	std::array vertices{
		  -0.5f,  0.5f,  0.0f,
		  -0.5f, -0.5f,  0.0f,
		   0.5f,  0.5f,  0.0f,
		   0.5f, -0.5f,  0.0f,
	};

	// clang-format on
	m_rectangleVertexBuffer
		= RBuffer::CreateTransfer("Billboard rectangle Vertex", vertices, vk::BufferUsageFlagBits::eVertexBuffer);
}
} // namespace vl
