#include "pch.h"
#include "renderer/VulkanLayer.h"

#include "editor/imgui/ImguiImpl.h"
#include "engine/Engine.h"
#include "engine/Events.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "renderer/asset/GpuAssetManager.h"
#include "renderer/wrapper/Device.h"
#include "world/nodes/camera/CameraNode.h"
#include "world/nodes/geometry/GeometryNode.h"
#include "world/World.h"

#include <array>

namespace {
vk::Extent2D SuggestFramebufferSize(vk::Extent2D viewportSize)
{
	return viewportSize;


	//// TODO:

	//// Viewport Framebuffer Size: (recommended framebuffer size)
	//// The size we actually want our framebuffers to allocate.
	//// This usually should be larger than the actual viewport size to allow seamless resizing (dynamic state) of the
	//// viewport while in the editor. In the real game this would always match the swapchain size and not waste any
	//// memory.
	//// The algorithm works like this:
	////   if: x <= 1920 & y <= 1080 -> framebufferSize == 1920x1080
	//// elif: x <= 2560 & y <= 1440 -> framebufferSize == 2560x1440
	//// elif: x <= 4096 & y <= 2160 -> framebufferSize == 4096x2160
	//// else: framebufferSize == viewport


	// vk::Extent2D sizes[] = {
	//	{ 1280, 800 },
	//	{ 1920, 1080 },
	//	{ 2560, 1440 },
	//	{ 4096, 2160 },
	//};
	// constexpr size_t sizesLen = sizeof(sizes) / sizeof(vk::Extent2D);

	// vk::Extent2D result = viewportSize;
	// for (size_t i = 0; i < sizesLen; i++) {
	//	if (sizes[i].width >= viewportSize.width) {
	//		result.width = sizes[i].width;
	//		break;
	//	}
	//}

	// for (size_t i = 0; i < sizesLen; i++) {
	//	if (sizes[i].height >= viewportSize.height) {
	//		result.height = sizes[i].height;
	//		break;
	//	}
	//}

	// return result;
}
} // namespace

VulkanLayer::VulkanLayer(std::vector<const char*>& extensions, GLFWwindow* window)
{
	// create vulkan instance with required extensions
	auto requiredExtensions = extensions;
	requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	instance = new Instance(requiredExtensions, window);

	// get first capable physical devices
	auto pd = instance->capablePhysicalDevices[0].get();

	// create logical device
	auto deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME };
	Device = new S_Device(pd, deviceExtensions);


	// create swapchain
	ReconstructSwapchain();
}

void VulkanLayer::Init()
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

	Event::OnViewportUpdated.Bind(this, [&] { didViewportResize.Set(); });
	Event::OnWindowResize.Bind(this, [&](auto, auto) { didWindowResize.Set(); });
	Event::OnWindowMinimize.Bind(this, [&](bool newIsMinimzed) { isMinimzed = newIsMinimzed; });


	Scene = new S_Scene();
}

VulkanLayer::~VulkanLayer()
{
	ImguiImpl::CleanupVulkan();
	GpuAssetManager.UnloadAll();
	delete Scene;
}

void VulkanLayer::ReconstructSwapchain()
{
	swapchain.reset();
	swapchain = std::make_unique<Swapchain>(instance->surface);
}


void VulkanLayer::InitModelDescriptors()
{
	// uniforms
	globalsUbo.reset(new Buffer(sizeof(UBO_Globals), vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));

	globalUboDescLayout.AddBinding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex);
	globalUboDescLayout.Generate();
	globalUboDescSet = globalUboDescLayout.GetDescriptorSet();
}

void VulkanLayer::InitDebugDescriptors()
{
	debugDescSetLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	debugDescSetLayout.Generate();
}

vk::UniqueSampler VulkanLayer::GetDefaultSampler()
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

void VulkanLayer::InitQuadDescriptor()
{
	for (uint32 i = 0u; i < 6u; ++i) {
		quadDescLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	}
	quadDescLayout.Generate();
	quadDescSet = quadDescLayout.GetDescriptorSet();

	quadSampler = GetDefaultSampler();
}

void VulkanLayer::UpdateQuadDescriptorSet()
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

void VulkanLayer::OnViewportResize()
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

void VulkanLayer::OnWindowResize()
{
	Device->waitIdle();
	ReconstructSwapchain();
}

void VulkanLayer::UpdateForFrame()
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
		if (Scene->cameras.elements.size()) {
			if (Scene->cameras.elements[0]) {
				ubo.viewProj = Scene->cameras.elements[0]->viewProj;
			}
		}

		globalsUbo->UploadData(&ubo, sizeof(ubo));
	}
}

void VulkanLayer::DrawGeometryPass(
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


void VulkanLayer::DrawDeferredPass(                 //
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
	renderPassInfo.setRenderPass(VulkanLayer::swapchain->renderPass.get()).setFramebuffer(framebuffer);
	renderPassInfo.renderArea
		.setOffset({ 0, 0 }) //
		.setExtent(VulkanLayer::swapchain->extent);

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

void VulkanLayer::DrawFrame()
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

	PROFILE_SCOPE(Renderer);

	vk::PresentInfoKHR presentInfo;
	presentInfo //
		.setWaitSemaphoreCount(0)
		.setPWaitSemaphores(nullptr);

	vk::SwapchainKHR swapChains[] = { swapchain->handle.get() };
	presentInfo.setSwapchainCount(1u).setPSwapchains(swapChains).setPImageIndices(&imageIndex).setPResults(nullptr);

	PROFILE_SCOPE(Renderer);
	Device->presentQueue.presentKHR(presentInfo);
	Device->waitIdle();
	// Device->waitIdle();
	Engine.m_sceneThreadFps.CountFrame();
}
