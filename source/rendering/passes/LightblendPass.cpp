#include "pch.h"
#include "LightblendPass.h"

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
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/SceneGeometry.h"
#include "rendering/passes/lightblend/DirlightBlend.h"
#include "rendering/passes/lightblend/PointlightBlend.h"
#include "rendering/passes/lightblend/ReflprobeBlend.h"
#include "rendering/passes/lightblend/SpotlightBlend.h"

#include <glm/gtc/matrix_inverse.hpp>

#include "assets/shared/GeometryShared.h"

namespace vl {
vk::UniqueRenderPass LightblendPass::CreateCompatibleRenderPass()
{
	vk::AttachmentDescription colorAttachment{};
	colorAttachment
		.setFormat(vk::Format::eR32G32B32A32Sfloat) //
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
		.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	vk::AttachmentReference colorAttachmentRef{};
	colorAttachmentRef
		.setAttachment(0u) //
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::SubpassDescription subpass{};
	subpass
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics) //
		.setColorAttachments(colorAttachmentRef);

	vk::SubpassDependency dependency{};
	dependency
		.setSrcSubpass(VK_SUBPASS_EXTERNAL) //
		.setDstSubpass(0u)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlags(0)) // 0
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo
		.setAttachments(colorAttachment) //
		.setSubpasses(subpass)
		.setDependencies(dependency);

	return Device->createRenderPassUnique(renderPassInfo);
}


void LightblendPass::RecordCmd(
	vk::CommandBuffer* cmdBuffer, vk::Viewport viewport, vk::Rect2D scissor, const SceneRenderDesc& sceneDesc)
{
	// vk::RenderPassBeginInfo renderPassInfo{};
	// renderPassInfo
	//	.setRenderPass(*Layouts->lightblendPass) //
	//	.setFramebuffer({});

	// renderPassInfo.renderArea
	//	.setOffset({ 0, 0 }) //
	//	.setExtent(sceneDesc.);

	// vk::ClearValue clearValue{};
	// clearValue.setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	// renderPassInfo
	//	.setClearValueCount(1u) //
	//	.setPClearValues(&clearValue);


	// cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	//{


	//	// cmdBuffer->beginRenderPass(beginInfo);

	//	cmdBuffer->setViewport(0, { viewport });
	//	cmdBuffer->setScissor(0, { scissor });

	//	// DirlightBlend::RecordCmd(*cmdBuffer, sceneDesc);
	//}
	// cmdBuffer->endRenderPass();
}

} // namespace vl
