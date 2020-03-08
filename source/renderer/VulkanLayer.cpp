#include "pch.h"

#include "renderer/VulkanLayer.h"
#include "engine/profiler/ProfileScope.h"

#include "engine/Engine.h"
#include "engine/Events.h"
#include "engine/Input.h"
#include "renderer/Model.h"

#include "world/World.h"
#include "world/nodes/camera/CameraNode.h"
#include "world/nodes/geometry/GeometryNode.h"
#include "editor/imgui/ImguiImpl.h"
#include <array>

namespace {
vk::Extent2D SuggestFramebufferSize(vk::Extent2D viewportSize)
{
	vk::Extent2D sizes[] = {
		{ 1280, 800 },
		{ 1920, 1080 },
		{ 2560, 1440 },
		{ 4096, 2160 },
	};
	constexpr size_t sizesLen = sizeof(sizes) / sizeof(vk::Extent2D);

	vk::Extent2D result = viewportSize;
	for (size_t i = 0; i < sizesLen; i++) {
		if (sizes[i].width >= viewportSize.width) {
			result.width = sizes[i].width;
			break;
		}
	}

	for (size_t i = 0; i < sizesLen; i++) {
		if (sizes[i].height >= viewportSize.height) {
			result.height = sizes[i].height;
			break;
		}
	}

	return result;
}
} // namespace


void VulkanLayer::InitVulkanLayer(std::vector<const char*>& extensions, GLFWwindow* window)
{
	// create vulkan instance with required extensions
	auto requiredExtensions = extensions;
	requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	instance = std::make_unique<Instance>(requiredExtensions, window);

	// get first capable physical devices
	auto pd = instance->capablePhysicalDevices[0].get();

	// create logical device
	auto deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME };
	device = std::make_unique<Device>(pd, deviceExtensions);


	// create swapchain
	ReconstructSwapchain();

	InitModelDescriptors();

	// CHECK: Code smell, needs internal first init function
	geomPass.InitRenderPass();
	geomPass.InitPipelineAndStuff();

	InitQuadDescriptor();
	OnViewportResize();

	defPass.InitPipeline(swapchain->renderPass.get());


	// NEXT: can be done with a single allocation
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setCommandPool(device->graphicsCmdPool.get())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);


	geometryCmdBuffer = std::move(device->allocateCommandBuffersUnique(allocInfo)[0]);


	vk::CommandBufferAllocateInfo allocInfo2{};
	allocInfo2.setCommandPool(device->graphicsCmdPool.get())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(static_cast<uint32>(swapchain->images.size()));

	outCmdBuffer = device->allocateCommandBuffersUnique(allocInfo2);


	imageAvailableSemaphore = device->createSemaphoreUnique({});
	renderFinishedSemaphore = device->createSemaphoreUnique({});

	Event::OnViewportUpdated.Bind(&eventHandler, [] { didViewportResize.Set(); });
	Event::OnWindowResize.Bind(&eventHandler, [](auto, auto) { didWindowResize.Set(); });
}

void VulkanLayer::ReconstructSwapchain()
{
	swapchain = std::make_unique<Swapchain>(device.get(), instance->surface);
}

void VulkanLayer::ReinitModels()
{
	auto world = Engine.GetWorld();
	models.clear();
	for (auto geomNode : world->GetNodeIterator<GeometryNode>()) {
		auto model = geomNode->GetModel();
		models.emplace_back(std::make_unique<Model>(model));
		models.back()->m_node = geomNode;
	}
}

void VulkanLayer::InitModelDescriptors()
{
	// uniforms

	vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

	device->CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, uniformBuffers,
		uniformBuffersMemory);


	// descriptor layout
	vk::DescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.setBinding(0u)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1u)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex)
		.setPImmutableSamplers(nullptr);

	vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.setBinding(1u)
		.setDescriptorCount(1u)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setPImmutableSamplers(nullptr)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	std::array<vk::DescriptorSetLayoutBinding, 2> bindings{ uboLayoutBinding, samplerLayoutBinding };
	vk::DescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.setBindingCount(static_cast<uint32>(bindings.size())).setPBindings(bindings.data());

	modelDescriptorSetLayout = device->createDescriptorSetLayoutUnique(layoutInfo);

	// TODO: RENDERER Global uniforms
	std::array<vk::DescriptorPoolSize, 2> poolSizes{};
	// for global uniforms
	poolSizes[0].setType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(1);
	// for image sampler combinations
	poolSizes[1].setType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1);

	vk::DescriptorPoolCreateInfo poolInfo{};
	poolInfo //
		.setPoolSizeCount(static_cast<uint32>(poolSizes.size()))
		.setPPoolSizes(poolSizes.data())
		.setMaxSets(500);

	modelDescriptorPool = device->createDescriptorPoolUnique(poolInfo);
}

vk::DescriptorSet VulkanLayer::GetModelDescriptorSet()
{
	vk::DescriptorSetAllocateInfo allocInfo{};

	allocInfo //
		.setDescriptorPool(modelDescriptorPool.get())
		.setDescriptorSetCount(1)
		.setPSetLayouts(&modelDescriptorSetLayout.get());

	// CHECK: are those destructed?
	return device->allocateDescriptorSets(allocInfo)[0];
}

void VulkanLayer::InitQuadDescriptor()
{
	// descriptor layout
	vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.setBinding(0u)
		.setDescriptorCount(1u)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setPImmutableSamplers(nullptr)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	vk::DescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.setBindingCount(1u).setPBindings(&samplerLayoutBinding);

	quadDescriptorSetLayout = device->createDescriptorSetLayoutUnique(layoutInfo);

	std::array<vk::DescriptorPoolSize, 1> poolSizes{};
	// for image sampler combinations
	poolSizes[0].setType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1);


	vk::DescriptorPoolCreateInfo poolInfo{};
	poolInfo //
		.setPoolSizeCount(static_cast<uint32>(poolSizes.size()))
		.setPPoolSizes(poolSizes.data())
		.setMaxSets(500); // TODO: GPU ASSETS

	quadDescriptorPool = device->createDescriptorPoolUnique(poolInfo);


	vk::DescriptorSetAllocateInfo allocInfo{};

	allocInfo //
		.setDescriptorPool(quadDescriptorPool.get())
		.setDescriptorSetCount(1)
		.setPSetLayouts(&quadDescriptorSetLayout.get());

	quadDescriptorSet = std::move(device->allocateDescriptorSetsUnique(allocInfo)[0]);

	// sampler
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo.setMagFilter(vk::Filter::eLinear)
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

	quadSampler = std::move(device->createSamplerUnique(samplerInfo));
}

void VulkanLayer::UpdateQuadDescriptorSet()
{

	vk::DescriptorImageInfo imageInfo{};
	imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setImageView(geomPass.albedoImageView.get())
		.setSampler(*quadSampler);

	vk::WriteDescriptorSet descriptorWrite{};
	descriptorWrite.setDstSet(quadDescriptorSet.get())
		.setDstBinding(0)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1u)
		.setPBufferInfo(nullptr)
		.setPImageInfo(&imageInfo)
		.setPTexelBufferView(nullptr);

	device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
}


void VulkanLayer::OnViewportResize()
{
	vk::Extent2D viewportSize{ g_ViewportCoordinates.size.x, g_ViewportCoordinates.size.y };

	viewportRect.extent = viewportSize;
	viewportRect.offset = vk::Offset2D(g_ViewportCoordinates.position.x, g_ViewportCoordinates.position.y);

	// NEXT: fix while keeping 1 to 1 ratio
	vk::Extent2D fbSize = SuggestFramebufferSize(viewportSize);
	// vk::Extent2D fbSize = viewportSize;

	if (fbSize != viewportFramebufferSize) {
		viewportFramebufferSize = fbSize;
		geomPass.InitFramebuffers();
		UpdateQuadDescriptorSet();
	}
}

void VulkanLayer::OnWindowResize()
{
	// NEXT:
}


void VulkanLayer::DrawFrame()
{
	PROFILE_SCOPE(Renderer);

	if (*didWindowResize) {
		OnWindowResize();
	}

	if (*didViewportResize) {
		OnViewportResize();
	}

	// NEXT: UNIFORM BUFFER UPDATES
	{
		auto world = Engine.GetWorld();
		auto camera = world->GetActiveCamera();

		UniformBufferObject ubo{};
		ubo.view = camera->GetViewMatrix();
		ubo.proj = camera->GetProjectionMatrix();

		void* data = device->mapMemory(uniformBuffersMemory.get(), 0, sizeof(ubo));
		memcpy(data, &ubo, sizeof(ubo));
		device->unmapMemory(uniformBuffersMemory.get());
	}

	// GEOMETRY PASS
	geomPass.TransitionGBufferForAttachmentWrite();

	geomPass.RecordGeometryDraw(&geometryCmdBuffer.get());

	vk::SubmitInfo submitInfo{};
	// vk::Semaphore waitSemaphores[] = { imageAvailableSemaphore.get() };
	// wait with writing colors to the image until it's available
	// the implementation can already start executing our vertex shader and such while the image is not yet
	// available
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	submitInfo.setWaitSemaphoreCount(0u)
		.setPWaitSemaphores(nullptr)
		.setPWaitDstStageMask(waitStages)
		.setCommandBufferCount(1u)
		.setPCommandBuffers(&geometryCmdBuffer.get());

	// which semaphores to signal once the command buffer(s) have finished execution
	// vk::Semaphore signalSemaphores[] = { m_renderFinishedSemaphore.get() };
	// submitInfo.setSignalSemaphoreCount(1u).setPSignalSemaphores(signalSemaphores);

	device->graphicsQueue.submit(1u, &submitInfo, {});
	device->graphicsQueue.waitIdle();

	// DEFERRED
	geomPass.TransitionGBufferForShaderRead();

	uint32 imageIndex;

	vk::Result result0 = device->acquireNextImageKHR(
		swapchain->handle.get(), UINT64_MAX, imageAvailableSemaphore.get(), {}, &imageIndex);

	switch (result0) {
		case vk::Result::eErrorOutOfDateKHR: return;
		case vk::Result::eSuccess:
		case vk::Result::eSuboptimalKHR: break;
		default: LOG_ABORT("failed to acquire swap chain image!");
	}

	auto cmdBuffer = &outCmdBuffer[imageIndex].get();
	auto& framebuffer = swapchain->framebuffers[imageIndex].get();
	{
		vk::CommandBufferBeginInfo beginInfo{};
		beginInfo.setFlags(vk::CommandBufferUsageFlags(0)).setPInheritanceInfo(nullptr);

		// begin command buffer recording
		cmdBuffer->begin(beginInfo);


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

		cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

		defPass.RecordCmd(cmdBuffer);
		editorPass.RecordCmd(cmdBuffer);

		cmdBuffer->endRenderPass();
		cmdBuffer->end();
	}


	vk::SubmitInfo submitInfo2{};
	vk::Semaphore waitSemaphores[] = { imageAvailableSemaphore.get() };
	vk::PipelineStageFlags waitStages2[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	submitInfo2.setWaitSemaphoreCount(0u)
		.setPWaitSemaphores(nullptr)
		.setPWaitDstStageMask(waitStages2)
		.setCommandBufferCount(1u)
		.setPCommandBuffers(&outCmdBuffer[imageIndex].get());

	device->graphicsQueue.submit(1u, &submitInfo2, {});
	device->graphicsQueue.waitIdle();


	vk::PresentInfoKHR presentInfo;
	presentInfo.setWaitSemaphoreCount(1u).setPWaitSemaphores(waitSemaphores);

	vk::SwapchainKHR swapChains[] = { swapchain->handle.get() };
	presentInfo.setSwapchainCount(1u).setPSwapchains(swapChains).setPImageIndices(&imageIndex).setPResults(nullptr);

	vk::Result result1 = device->presentQueue.presentKHR(presentInfo);

	device->presentQueue.waitIdle();

	switch (result1) {
		case vk::Result::eErrorOutOfDateKHR:
		case vk::Result::eSuboptimalKHR: return;
		case vk::Result::eSuccess: break;
		default: LOG_ABORT("failed to acquire swap chain image!");
	}
}
