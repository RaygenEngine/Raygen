#include "pch/pch.h"

#include "renderer/VulkanLayer.h"

#include "system/Engine.h"
#include "system/EngineEvents.h"
#include "system/Input.h"
#include "renderer/Model.h"
#include "world/World.h"
#include "world/nodes/camera/CameraNode.h"
#include "world/nodes/geometry/GeometryNode.h"
#include "editor/imgui/ImguiImpl.h"


void VulkanLayer::InitVulkanLayer(std::vector<const char*>& extensions, WindowType* window)
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
	device = std::make_unique<LogicalDevice>(pd, deviceExtensions);

	// create swapchain
	ReconstructSwapchain();

	InitModelDescriptors();

	geomPass.InitRenderPassAndFramebuffers();
	geomPass.InitPipelineAndStuff();


	InitQuadDescriptor();

	defPass.InitRenderPassAndFramebuffers();
	defPass.InitPipelineAndStuff();


	editorPass.InitRenderPassAndFramebuffers();

	// WIP
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setCommandPool(device->graphicsCmdPool.get())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);


	geometryCmdBuffer = std::move(device->handle->allocateCommandBuffersUnique(allocInfo)[0]);


	vk::CommandBufferAllocateInfo allocInfo2{};
	allocInfo2.setCommandPool(device->graphicsCmdPool.get())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(static_cast<uint32>(swapchain->images.size()));

	outCmdBuffer = device->handle->allocateCommandBuffersUnique(allocInfo2);


	imageAvailableSemaphore = device->handle->createSemaphoreUnique({});
	renderFinishedSemaphore = device->handle->createSemaphoreUnique({});
}

void VulkanLayer::ReconstructSwapchain()
{
	swapchain = std::make_unique<Swapchain>(device.get(), instance->surface);
}


void VulkanLayer::ReinitModels()
{
	auto world = Engine::GetWorld();
	models.clear();
	for (auto geomNode : world->GetNodeIterator<GeometryNode>()) {
		auto model = geomNode->GetModel();
		models.emplace_back(std::make_unique<Model>(model)); // TODO: RENDERER
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

	modelDescriptorSetLayout = device->handle->createDescriptorSetLayoutUnique(layoutInfo);

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

	modelDescriptorPool = device->handle->createDescriptorPoolUnique(poolInfo);
}

vk::DescriptorSet VulkanLayer::GetModelDescriptorSet()
{
	vk::DescriptorSetAllocateInfo allocInfo{};

	allocInfo //
		.setDescriptorPool(modelDescriptorPool.get())
		.setDescriptorSetCount(1)
		.setPSetLayouts(&modelDescriptorSetLayout.get());

	// WIP: are those destructed?
	return device->handle->allocateDescriptorSets(allocInfo)[0];
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

	quadDescriptorSetLayout = device->handle->createDescriptorSetLayoutUnique(layoutInfo);

	// WIP: Global uniforms
	std::array<vk::DescriptorPoolSize, 1> poolSizes{};
	// for image sampler combinations
	poolSizes[0].setType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1);


	vk::DescriptorPoolCreateInfo poolInfo{};
	poolInfo //
		.setPoolSizeCount(static_cast<uint32>(poolSizes.size()))
		.setPPoolSizes(poolSizes.data())
		.setMaxSets(500); // WIP

	quadDescriptorPool = device->handle->createDescriptorPoolUnique(poolInfo);


	vk::DescriptorSetAllocateInfo allocInfo{};

	allocInfo //
		.setDescriptorPool(quadDescriptorPool.get())
		.setDescriptorSetCount(1)
		.setPSetLayouts(&quadDescriptorSetLayout.get());

	// WIP: are those destructed?
	quadDescriptorSet = std::move(device->handle->allocateDescriptorSetsUnique(allocInfo)[0]);

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

	static auto sampler = device->handle->createSamplerUnique(samplerInfo);

	vk::DescriptorImageInfo imageInfo{};
	imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setImageView(geomPass.albedoImageView.get())
		.setSampler(sampler.get());

	vk::WriteDescriptorSet descriptorWrite{};
	descriptorWrite.setDstSet(quadDescriptorSet.get())
		.setDstBinding(0)
		.setDstArrayElement(0)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1u)
		.setPBufferInfo(nullptr)
		.setPImageInfo(&imageInfo)
		.setPTexelBufferView(nullptr);

	device->handle->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
}


void VulkanLayer::DrawFrame()
{

	// WIP: UNIFORM BUFFER UPDATES
	{
		auto world = Engine::GetWorld();
		auto camera = world->GetActiveCamera();

		UniformBufferObject ubo{};
		ubo.view = camera->GetViewMatrix();
		ubo.proj = camera->GetProjectionMatrix();

		void* data = device->handle->mapMemory(uniformBuffersMemory.get(), 0, sizeof(ubo));
		memcpy(data, &ubo, sizeof(ubo));
		device->handle->unmapMemory(uniformBuffersMemory.get());
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

	device->graphicsQueue.handle.submit(1u, &submitInfo, {});

	device->graphicsQueue.handle.waitIdle();

	// DEFERRED
	geomPass.TransitionGBufferForShaderRead();

	uint32 imageIndex;

	vk::Result result0 = device->handle->acquireNextImageKHR(
		swapchain->handle.get(), UINT64_MAX, imageAvailableSemaphore.get(), {}, &imageIndex);

	switch (result0) {
		case vk::Result::eErrorOutOfDateKHR: return;
		case vk::Result::eSuccess:
		case vk::Result::eSuboptimalKHR: break;
		default: LOG_ABORT("failed to acquire swap chain image!");
	}


	// WIP:
	static bool def{ false };
	if (Engine::GetInput().IsJustPressed(Key::B)) {
		def = !def;
	}

	if (def) {
		defPass.RecordCmd(&outCmdBuffer[imageIndex].get(), swapchain->framebuffers[imageIndex].get());
	}
	else {
		editorPass.RecordCmd(&outCmdBuffer[imageIndex].get(), swapchain->framebuffers[imageIndex].get());
	}


	vk::SubmitInfo submitInfo2{};
	vk::Semaphore waitSemaphores[] = { imageAvailableSemaphore.get() };
	vk::PipelineStageFlags waitStages2[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	submitInfo2.setWaitSemaphoreCount(0u)
		.setPWaitSemaphores(nullptr)
		.setPWaitDstStageMask(waitStages2)
		.setCommandBufferCount(1u)
		.setPCommandBuffers(&outCmdBuffer[imageIndex].get());

	device->graphicsQueue.handle.submit(1u, &submitInfo2, {});
	device->graphicsQueue.handle.waitIdle();


	vk::PresentInfoKHR presentInfo;
	presentInfo.setWaitSemaphoreCount(1u).setPWaitSemaphores(waitSemaphores);

	vk::SwapchainKHR swapChains[] = { swapchain->handle.get() };
	presentInfo.setSwapchainCount(1u).setPSwapchains(swapChains).setPImageIndices(&imageIndex).setPResults(nullptr);

	vk::Result result1 = device->presentQueue.handle.presentKHR(presentInfo);

	device->presentQueue.handle.waitIdle();

	switch (result1) {
		case vk::Result::eErrorOutOfDateKHR:
		case vk::Result::eSuboptimalKHR: return;
		case vk::Result::eSuccess: break;
		default: LOG_ABORT("failed to acquire swap chain image!");
	}
}
