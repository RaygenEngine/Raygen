#include "pch.h"
#include "GBufferPass.h"

#include "engine/Engine.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Device.h"
#include "rendering/Renderer.h"
#include "rendering/scene/Scene.h"
#include "rendering/Layouts.h"
#include "rendering/objects/GBuffer.h"

#include <glm/gtc/matrix_inverse.hpp>

namespace {
struct PushConstant {
	glm::mat4 modelMat;
	glm::mat4 normalMat;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
GBufferPass::GBufferPass()
{
	// renderpass
	std::array<vk::AttachmentDescription, 5> colorAttachmentDescs{};
	std::array<vk::AttachmentReference, 5> colorAttachmentRefs{};

	for (size_t i = 0; i < 5; ++i) {
		colorAttachmentDescs[i]
			.setFormat(GBuffer::colorAttachmentFormats[i]) // CHECK:
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(
				vk::ImageLayout::eColorAttachmentOptimal)             // CHECK: vk::ImageLayout::eShaderReadOnlyOptimal?
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // CHECK:

		colorAttachmentRefs[i]
			.setAttachment(static_cast<uint32>(i)) //
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	}

	vk::AttachmentDescription depthAttachmentDesc{};
	depthAttachmentDesc
		.setFormat(Device->pd->FindDepthFormat()) // CHECK:
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare) // CHECK: if use stencil dont forget those two
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(
			vk::ImageLayout::eDepthStencilAttachmentOptimal)      // CHECK: vk::ImageLayout::eShaderReadOnlyOptimal?
		.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // CHECK:

	vk::AttachmentReference depthAttachmentRef{};
	depthAttachmentRef
		.setAttachment(5u) //
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

	std::array attachments{ colorAttachmentDescs[0], colorAttachmentDescs[1], colorAttachmentDescs[2],
		colorAttachmentDescs[3], colorAttachmentDescs[4], depthAttachmentDesc };
	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo
		.setAttachmentCount(static_cast<uint32>(attachments.size())) //
		.setPAttachments(attachments.data())
		.setSubpassCount(1u)
		.setPSubpasses(&subpass)
		.setDependencyCount(1u)
		.setPDependencies(&dependency);

	m_renderPass = Device->createRenderPassUnique(renderPassInfo);

	// pipeline layout
	vk::PushConstantRange pushConstantRange{};
	pushConstantRange
		.setStageFlags(vk::ShaderStageFlagBits::eVertex) //
		.setSize(sizeof(PushConstant))
		.setOffset(0u);

	std::array layouts
		= { Layouts->regularMaterialDescLayout.setLayout.get(), Layouts->singleUboDescLayout.setLayout.get() };

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setSetLayoutCount(static_cast<uint32>(layouts.size())) //
		.setPSetLayouts(layouts.data())
		.setPushConstantRangeCount(1u)
		.setPPushConstantRanges(&pushConstantRange);

	m_pipelineLayout = Device->createPipelineLayoutUnique(pipelineLayoutInfo);
}

void GBufferPass::RecordCmd(
	vk::CommandBuffer* cmdBuffer, GBuffer* gBuffer, const std::vector<SceneGeometry*>& geometries)
{
	PROFILE_SCOPE(Renderer);

	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo
		.setRenderPass(m_renderPass.get()) //
		.setFramebuffer(gBuffer->GetFramebuffer());
	renderPassInfo.renderArea
		.setOffset({ 0, 0 }) //
		.setExtent(gBuffer->GetExtent());

	std::array<vk::ClearValue, 6> clearValues = {};
	clearValues[0].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[1].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[2].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[3].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[4].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[5].setDepthStencil({ 1.0f, 0 });
	renderPassInfo
		.setClearValueCount(static_cast<uint32>(clearValues.size())) //
		.setPClearValues(clearValues.data());


	cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	{
		// TODO: remove camera refs
		auto camera = Scene->GetActiveCamera();
		if (!camera) {
			cmdBuffer->endRenderPass();
			return;
		}

		for (auto geom : geometries) {
			PushConstant pc{ //
				geom->transform, glm::inverseTranspose(glm::mat3(geom->transform))
			};

			for (auto& gg : geom->model.Lock().geometryGroups) {
				auto& mat = gg.material.Lock();

				if (!mat.wip_CustomOverride) {
					// Old regular material.
					cmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, gBuffer->GetPipeline());

					cmdBuffer->pushConstants(
						m_pipelineLayout.get(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);


					cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 1u, 1u,
						&Scene->GetActiveCameraDescSet(), 0u, nullptr);

					cmdBuffer->bindVertexBuffers(0u, { *gg.vertexBuffer }, { 0 });
					cmdBuffer->bindIndexBuffer(*gg.indexBuffer, 0, vk::IndexType::eUint32);

					cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, 1u,
						&gg.material.Lock().descriptorSet, 0u, nullptr);

					cmdBuffer->drawIndexed(gg.indexCount, 1u, 0u, 0u, 0u);
				}
				else {

					auto& matNew = mat.wip_New;


					cmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *matNew.pipeline);


					cmdBuffer->pushConstants(
						*matNew.plLayout, vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);


					cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *matNew.plLayout, 1u, 1u,
						&Scene->GetActiveCameraDescSet(), 0u, nullptr);

					cmdBuffer->bindVertexBuffers(0u, { *gg.vertexBuffer }, { 0 });
					cmdBuffer->bindIndexBuffer(*gg.indexBuffer, 0, vk::IndexType::eUint32);

					cmdBuffer->bindDescriptorSets(
						vk::PipelineBindPoint::eGraphics, *matNew.plLayout, 0u, 1u, &matNew.descSet, 0u, nullptr);

					cmdBuffer->drawIndexed(gg.indexCount, 1u, 0u, 0u, 0u);
				}
			}
		}
	}
	cmdBuffer->endRenderPass();
}

UniquePtr<GBuffer> GBufferPass::CreateCompatibleGBuffer(uint32 width, uint32 height)
{
	return std::make_unique<GBuffer>(this, width, height);
}
} // namespace vl
