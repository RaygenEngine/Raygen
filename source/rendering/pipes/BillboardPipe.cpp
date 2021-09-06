#include "BillboardPipe.h"

#include "editor/Editor.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/SceneCamera.h"
#include "universe/components/CameraComponent.h"
#include "universe/components/DirlightComponent.h"
#include "universe/components/IrragridComponent.h"
#include "universe/components/PointlightComponent.h"
#include "universe/components/QuadlightComponent.h"
#include "universe/components/ReflProbeComponent.h"
#include "universe/components/SpotlightComponent.h"
#include "universe/Universe.h"

namespace {
struct PushConstant {
	glm::mat4 viewProj;
	glm::vec4 position;
	glm::vec4 cameraRight;
	glm::vec4 cameraUp;
	glm::vec2 uvMin{ 0.f, 0.f };
	glm::vec2 uvMax{ 1.f, 1.f };
};

static_assert(sizeof(PushConstant) <= 128);

// The VBO containing the 4 vertices of the particles.
} // namespace

namespace vl {

vk::UniquePipelineLayout BillboardPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayout<PushConstant>({
		DescriptorLayouts->_1imageSamplerFragmentOnly.handle(),
	});
}

vk::UniquePipeline BillboardPipe::MakePipeline()
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/unlit/billboard.shader");
	gpuShader.onCompile = [&]() {
		StaticPipes::Recompile<BillboardPipe>();
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
	vk::PipelineVertexInputStateCreateInfo vertexInputState{};
	vertexInputState
		.setVertexBindingDescriptions(bindingDescription) //
		.setVertexAttributeDescriptions(attributeDescription);

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState{};
	inputAssemblyState
		.setTopology(vk::PrimitiveTopology::eTriangleStrip) //
		.setPrimitiveRestartEnable(VK_FALSE);

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

	// depth and stencil state
	vk::PipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState
		.setDepthTestEnable(VK_TRUE)  //
		.setDepthWriteEnable(VK_TRUE) // CHECK: write to depth, this is probably the last pass so we don't care for the
									  // state of the depth buffer
		.setDepthCompareOp(vk::CompareOp::eLess)
		.setDepthBoundsTestEnable(VK_FALSE)
		.setMinDepthBounds(0.0f) // Optional
		.setMaxDepthBounds(1.0f) // Optional
		.setStencilTestEnable(VK_FALSE)
		.setFront({}) // Optional
		.setBack({}); // Optional

	return rvk::makeGraphicsPipeline(gpuShader.shaderStages, &vertexInputState, &inputAssemblyState, nullptr, nullptr,
		&rasterizer, nullptr, &depthStencilState, nullptr, nullptr, layout(),
		PassLayouts->unlit.compatibleRenderPass.get(), 0u);
}

namespace {
	// Unoptimized, we should do views & ubo batching, but we also need to keep track of double draws or iterate per
	// entity and draw first Component found from list (slow), we should also do basic visiblity testing (eg behind view
	// plane, or too far, maybe even frustum testing)
	template<CComponent T>
	void DrawComponent(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, vk::PipelineLayout layout)
	{
		auto view = sceneDesc.viewer.ubo.view;
		glm::vec4 cameraRight{ view[0][0], view[1][0], view[2][0], 0.f };
		glm::vec4 cameraUp{ view[0][1], view[1][1], view[2][1], 0.f };

		auto icon = ComponentsDb::GetType<T>()->clPtr->GetIcon();
		auto uv = Editor::GetIconUV(U8(icon));

		for (auto&& [ent, comp, bc] : Universe::MainWorld->GetView<T, BasicComponent>().each()) {

			PushConstant pc{
				.viewProj = sceneDesc.viewer.ubo.viewProj,
				.position = glm::vec4(bc.world().position, 1.f),
				.cameraRight = cameraRight,
				.cameraUp = cameraUp,
				.uvMin = uv.first,
				.uvMax = uv.second,
			};

			cmdBuffer.pushConstants(layout, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0u,
				sizeof(PushConstant), &pc);

			rvk::drawUnitRectTriangleStrip(cmdBuffer);
		}
	};


	template<>
	void DrawComponent<CIrragrid>(
		vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, vk::PipelineLayout layout)
	{
		auto view = sceneDesc.viewer.ubo.view;
		glm::vec4 cameraRight{ view[0][0], view[1][0], view[2][0], 0.f };
		glm::vec4 cameraUp{ view[0][1], view[1][1], view[2][1], 0.f };

		auto icon = ComponentsDb::GetType<CIrragrid>()->clPtr->GetIcon();
		auto uv = Editor::GetIconUV(U8(icon));


		for (auto&& [ent, ig, bc] : Universe::MainWorld->GetView<CIrragrid, BasicComponent>().each()) {

			if (ig.hideBillboards) {
				continue;
			}

			for (int32 x = 0; x < ig.width; ++x) {
				for (int32 y = 0; y < ig.height; ++y) {
					for (int32 z = 0; z < ig.depth; ++z) {
						auto pos = bc.world().position + glm::vec3(x, y, z) * ig.distToAdjacent;

						PushConstant pc{
							.viewProj = sceneDesc.viewer.ubo.viewProj,
							.position = glm::vec4(pos, 1.f),
							.cameraRight = cameraRight,
							.cameraUp = cameraUp,
							.uvMin = uv.first,
							.uvMax = uv.second,
						};

						cmdBuffer.pushConstants(layout,
							vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0u,
							sizeof(PushConstant), &pc);

						rvk::drawUnitRectTriangleStrip(cmdBuffer);
					}
				}
			}
		}
	};
} // namespace

void BillboardPipe::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc) const
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline());
	rvk::bindUnitRectTriangleStrip(cmdBuffer);

	auto fontDescSet = static_cast<VkDescriptorSet>(Editor::GetFontIconTexture());
	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, layout(), 0u, { fontDescSet }, nullptr);

	DrawComponent<CReflprobe>(cmdBuffer, sceneDesc, layout());
	DrawComponent<CPointlight>(cmdBuffer, sceneDesc, layout());
	DrawComponent<CSpotlight>(cmdBuffer, sceneDesc, layout());
	DrawComponent<CDirlight>(cmdBuffer, sceneDesc, layout());
	DrawComponent<CIrragrid>(cmdBuffer, sceneDesc, layout());
	DrawComponent<CCamera>(cmdBuffer, sceneDesc, layout());
	DrawComponent<CQuadlight>(cmdBuffer, sceneDesc, layout());
}

} // namespace vl
