#include "pch.h"
#include "Renderer.h"

#include "editor/imgui/ImguiImpl.h"
#include "engine/Engine.h"
#include "engine/Events.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/asset/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Instance.h"
#include "rendering/wrapper/Swapchain.h"
#include "universe/nodes/camera/CameraNode.h"
#include "universe/nodes/geometry/GeometryNode.h"
#include "universe/World.h"

#include <array>

namespace {
vk::Extent2D SuggestFramebufferSize(vk::Extent2D viewportSize)
{
	return viewportSize;
}
} // namespace

namespace vl {

Renderer_::Renderer_()
{
	Scene = new Scene_();

	// create swapchain
	ReconstructSwapchain();
}

void Renderer_::Init()
{
	InitModelDescriptors();

	// CHECK: Code smell, needs internal first init function
	geomPass.InitAll();

	InitQuadDescriptor();
	InitDebugDescriptors();
	OnViewportResize();

	defPass.InitPipeline(swapchain->renderPass.get());


	// NEXT: can be done with a single allocation
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setCommandPool(Device->graphicsCmdPool.get())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);


	geometryCmdBuffer = Device->allocateCommandBuffers(allocInfo)[0];


	vk::CommandBufferAllocateInfo allocInfo2{};
	allocInfo2.setCommandPool(Device->graphicsCmdPool.get())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(static_cast<uint32>(swapchain->images.size()));

	outCmdBuffer = Device->allocateCommandBuffers(allocInfo2);


	renderFinishedSemaphore = Device->createSemaphoreUnique({});
	imageAcquiredSem = Device->createSemaphoreUnique({});


	Event::OnViewportUpdated.BindFlag(this, didViewportResize);
	Event::OnWindowResize.BindFlag(this, didWindowResize);
	Event::OnWindowMinimize.Bind(this, [&](bool newIsMinimzed) { isMinimzed = newIsMinimzed; });
}

Renderer_::~Renderer_()
{
	delete swapchain;

	delete Scene;
}

void Renderer_::ReconstructSwapchain()
{
	delete swapchain;
	swapchain = new Swapchain(Instance->surface);
}


void Renderer_::InitModelDescriptors()
{
	// uniforms
	globalsUbo.reset(new Buffer(sizeof(UBO_Globals), vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));

	globalUboDescLayout.AddBinding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex);
	globalUboDescLayout.Generate();
	globalUboDescSet = globalUboDescLayout.GetDescriptorSet();
}

void Renderer_::InitDebugDescriptors()
{
	debugDescSetLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	debugDescSetLayout.Generate();
}

vk::UniqueSampler Renderer_::GetDefaultSampler()
{
	// CHECK: same for all?
	// sampler
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo
		.setMagFilter(vk::Filter::eLinear) //
		.setMinFilter(vk::Filter::eLinear)
		.setAddressModeU(vk::SamplerAddressMode::eRepeat)
		.setAddressModeV(vk::SamplerAddressMode::eRepeat)
		.setAddressModeW(vk::SamplerAddressMode::eRepeat)
		// PERF:
		.setAnisotropyEnable(VK_FALSE)
		.setMaxAnisotropy(1u)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setUnnormalizedCoordinates(VK_FALSE)
		.setCompareEnable(VK_FALSE)
		.setCompareOp(vk::CompareOp::eAlways)
		.setMipmapMode(vk::SamplerMipmapMode::eLinear)
		.setMipLodBias(0.f)
		.setMinLod(0.f)
		.setMaxLod(0.f);

	return Device->createSamplerUnique(samplerInfo);
}

void Renderer_::InitQuadDescriptor()
{
	for (uint32 i = 0u; i < 6u; ++i) {
		quadDescLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	}
	quadDescLayout.Generate();
	quadDescSet = quadDescLayout.GetDescriptorSet();

	quadSampler = GetDefaultSampler();
}

void Renderer_::UpdateQuadDescriptorSet()
{
	auto UpdateImageSamplerInDescriptorSet = [&](vk::ImageView& view, vk::Sampler& sampler, uint32 dstBinding) {
		vk::DescriptorImageInfo imageInfo{};
		imageInfo
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
			.setImageView(view)
			.setSampler(sampler);

		vk::WriteDescriptorSet descriptorWrite{};
		descriptorWrite
			.setDstSet(quadDescSet) //
			.setDstBinding(dstBinding)
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1u)
			.setPBufferInfo(nullptr)
			.setPImageInfo(&imageInfo)
			.setPTexelBufferView(nullptr);

		Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
	};

	UpdateImageSamplerInDescriptorSet(geomPass.m_gBuffer->position->view.get(), quadSampler.get(), 0u);
	UpdateImageSamplerInDescriptorSet(geomPass.m_gBuffer->normal->view.get(), quadSampler.get(), 1u);
	UpdateImageSamplerInDescriptorSet(geomPass.m_gBuffer->albedo->view.get(), quadSampler.get(), 2u);
	UpdateImageSamplerInDescriptorSet(geomPass.m_gBuffer->specular->view.get(), quadSampler.get(), 3u);
	UpdateImageSamplerInDescriptorSet(geomPass.m_gBuffer->emissive->view.get(), quadSampler.get(), 4u);
	UpdateImageSamplerInDescriptorSet(geomPass.m_gBuffer->depth->view.get(), quadSampler.get(), 5u); // NEXT:


	vk::DescriptorBufferInfo bufferInfo{};

	bufferInfo
		.setBuffer(*globalsUbo) //
		.setOffset(0u)
		.setRange(sizeof(UBO_Globals));
	vk::WriteDescriptorSet descriptorWrite{};

	descriptorWrite
		.setDstSet(globalUboDescSet) //
		.setDstBinding(0u)
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1u)
		.setPBufferInfo(&bufferInfo)
		.setPImageInfo(nullptr)
		.setPTexelBufferView(nullptr);

	Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
}

void Renderer_::OnViewportResize()
{
	vk::Extent2D viewportSize{ g_ViewportCoordinates.size.x, g_ViewportCoordinates.size.y };

	viewportRect.extent = viewportSize;
	viewportRect.offset = vk::Offset2D(g_ViewportCoordinates.position.x, g_ViewportCoordinates.position.y);

	vk::Extent2D fbSize = SuggestFramebufferSize(viewportSize);

	if (fbSize != viewportFramebufferSize) {
		viewportFramebufferSize = fbSize;
		geomPass.InitFramebuffers();
		UpdateQuadDescriptorSet();
	}
}

void Renderer_::OnWindowResize()
{
	Device->waitIdle();
	ReconstructSwapchain();
}

void Renderer_::UpdateForFrame()
{
	if (*didWindowResize) {
		OnWindowResize();
	}

	if (*didViewportResize) {
		OnViewportResize();
	}

	Scene->ConsumeCmdQueue();


	// Globals buffer updates (one uniform buffer update for all draw calls of this render pass)
	{
		UBO_Globals ubo{};
		if (Scene->cameras.elements.size() > Scene->activeCamera) {
			auto cam = Scene->cameras.elements[Scene->activeCamera];
			if (cam) {
				ubo.viewProj = cam->viewProj;
			}
		}

		globalsUbo->UploadData(&ubo, sizeof(ubo));
	}
}

void Renderer_::DrawGeometryPass(
	std::vector<vk::PipelineStageFlags> waitStages, SemVec waitSemaphores, SemVec signalSemaphores)
{

	geomPass.RecordGeometryDraw(&geometryCmdBuffer);

	vk::SubmitInfo submitInfo{};
	submitInfo
		.setWaitSemaphoreCount(static_cast<uint32>(waitSemaphores.size())) //
		.setPWaitSemaphores(waitSemaphores.data())
		.setPWaitDstStageMask(waitStages.data())

		.setSignalSemaphoreCount(static_cast<uint32>(signalSemaphores.size()))
		.setPSignalSemaphores(signalSemaphores.data())

		.setCommandBufferCount(1u)
		.setPCommandBuffers(&geometryCmdBuffer);

	Device->graphicsQueue.submit(1u, &submitInfo, {});
}


void Renderer_::DrawDeferredPass(                   //
	std::vector<vk::PipelineStageFlags> waitStages, //
	SemVec waitSemaphores,                          //
	SemVec signalSemaphores,                        //
	vk::CommandBuffer cmdBuffer,                    //
	vk::Framebuffer framebuffer)
{
	PROFILE_SCOPE(Renderer);

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlags(0)).setPInheritanceInfo(nullptr);

	// begin command buffer recording
	cmdBuffer.begin(beginInfo);


	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo.setRenderPass(swapchain->renderPass.get()).setFramebuffer(framebuffer);
	renderPassInfo.renderArea
		.setOffset({ 0, 0 }) //
		.setExtent(swapchain->extent);

	std::array<vk::ClearValue, 2> clearValues = {};
	clearValues[0].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[1].setDepthStencil({ 1.0f, 0 });
	renderPassInfo.setClearValueCount(static_cast<uint32>(clearValues.size()));
	renderPassInfo.setPClearValues(clearValues.data());


	cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

	defPass.RecordCmd(&cmdBuffer);
	editorPass.RecordCmd(&cmdBuffer);

	cmdBuffer.endRenderPass();
	geomPass.m_gBuffer->TransitionForAttachmentWrite(cmdBuffer);

	cmdBuffer.end();


	vk::SubmitInfo submitInfo{};
	submitInfo
		.setWaitSemaphoreCount(static_cast<uint32>(waitSemaphores.size())) //
		.setPWaitSemaphores(waitSemaphores.data())
		.setPWaitDstStageMask(waitStages.data())

		.setSignalSemaphoreCount(static_cast<uint32>(signalSemaphores.size()))
		.setPSignalSemaphores(signalSemaphores.data())

		.setCommandBufferCount(1u)
		.setPCommandBuffers(&cmdBuffer);

	Device->graphicsQueue.submit(1u, &submitInfo, {});
}

void Renderer_::DrawFrame()
{
	if (isMinimzed) {
		return;
	}

	PROFILE_SCOPE(Renderer);

	UpdateForFrame();

	// DrawGeometryPass({}, {}, { *gbufferReadySem });
	DrawGeometryPass({}, {}, {});

	// DEFERRED
	uint32 imageIndex;
	Device->acquireNextImageKHR(swapchain->handle.get(), UINT64_MAX, { *imageAcquiredSem }, {}, &imageIndex);

	DrawDeferredPass(
		//
		{ vk::PipelineStageFlagBits::eColorAttachmentOutput }, //
		{ *imageAcquiredSem },                                 //
		{},                                                    //
		outCmdBuffer[imageIndex],
		swapchain->framebuffers[imageIndex].get() //
	);

	vk::PresentInfoKHR presentInfo;
	presentInfo //
		.setWaitSemaphoreCount(0)
		.setPWaitSemaphores(nullptr);

	vk::SwapchainKHR swapChains[] = { swapchain->handle.get() };
	presentInfo //
		.setSwapchainCount(1u)
		.setPSwapchains(swapChains)
		.setPImageIndices(&imageIndex)
		.setPResults(nullptr);

	PROFILE_SCOPE(Renderer);
	Device->presentQueue.presentKHR(presentInfo);
	Device->waitIdle();
}
} // namespace vl
