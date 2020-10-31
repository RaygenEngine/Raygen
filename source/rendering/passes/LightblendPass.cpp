#include "LightblendPass.h"


namespace vl {

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
