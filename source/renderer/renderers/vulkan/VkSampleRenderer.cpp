#include "pch/pch.h"

#include "renderer/renderers/vulkan/VkSampleRenderer.h"
#include "system/Logger.h"
#include "system/Engine.h"

#include "asset/AssetManager.h"
#include "world/World.h"
#include "world/nodes/geometry/GeometryNode.h"
#include "world/nodes/camera/CameraNode.h"
#include "platform/windows/Win32Window.h"

#include <set>

#include <vulkan/vulkan_win32.h>


constexpr bool useSeperateTransferQueue = false;

namespace {
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(
	VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	switch (messageSeverity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			LOG_DEBUG("Validation layer: {}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			LOG_INFO("Validation layer: {}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			LOG_WARN("Validation layer: {}", pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			LOG_ERROR("Validation layer: {}", pCallbackData->pMessage);
			break;
		default: break;
	}

	return VK_FALSE;
}

void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
								 | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
								 | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
							 | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
							 | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
}

std::vector<const char*> requiredExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
} // namespace
namespace vlkn {

VkSampleRenderer::~VkSampleRenderer()
{
	Engine::Get().m_remakeWindow = true;
}

void VkSampleRenderer::CreateRenderCommandBuffers()
{
	auto world = Engine::GetWorld();
	auto geomNode = world->GetAnyAvailableNode<GeometryNode>();
	auto model = geomNode->GetModel();
	m_model = std::make_unique<vlkn::Model>(m_device.get(), model);


	m_renderCommandBuffers.resize(m_swapchain->GetImageCount());

	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setCommandPool(m_device->GetGraphicsCommandPool())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(static_cast<uint32>(m_renderCommandBuffers.size()));

	m_renderCommandBuffers = m_device->allocateCommandBuffers(allocInfo);

	for (int32 i = 0; i < m_renderCommandBuffers.size(); ++i) {
		vk::CommandBufferBeginInfo beginInfo{};
		beginInfo.setFlags(vk::CommandBufferUsageFlags(0)).setPInheritanceInfo(nullptr);

		// begin command buffer recording
		m_renderCommandBuffers[i].begin(beginInfo);
		{
			vk::RenderPassBeginInfo renderPassInfo{};
			renderPassInfo.setRenderPass(m_swapchain->GetRenderPass())
				.setFramebuffer(m_swapchain->GetFramebuffers()[i]);
			renderPassInfo.renderArea.setOffset({ 0, 0 }).setExtent(m_swapchain->GetExtent());
			std::array<vk::ClearValue, 2> clearValues = {};
			clearValues[0].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
			clearValues[1].setDepthStencil({ 1.0f, 0 });
			renderPassInfo.setClearValueCount(static_cast<uint32>(clearValues.size()));
			renderPassInfo.setPClearValues(clearValues.data());

			// begin render pass
			m_renderCommandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
			{
				// bind the graphics pipeline
				m_renderCommandBuffers[i].bindPipeline(
					vk::PipelineBindPoint::eGraphics, m_graphicsPipeline->GetPipeline());

				for (auto& gg : m_model->GetGeometryGroups()) {
					vk::Buffer vertexBuffers[] = { gg.vertexBuffer.get() };
					vk::DeviceSize offsets[] = { 0 };
					// geom
					m_renderCommandBuffers[i].bindVertexBuffers(0u, 1u, vertexBuffers, offsets);

					// indices
					m_renderCommandBuffers[i].bindIndexBuffer(gg.indexBuffer.get(), 0, vk::IndexType::eUint32);

					// descriptor sets
					m_renderCommandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
						m_graphicsPipeline->GetPipelineLayout(), 0u, 1u, &(m_descriptors->GetDescriptorSets()[i]), 0u,
						nullptr);

					// draw call (triangle)
					m_renderCommandBuffers[i].drawIndexed(gg.indexCount, 1u, 0u, 0u, 0u);
				}
			}
			// end render pass
			m_renderCommandBuffers[i].endRenderPass();
		}
		// end command buffer recording
		m_renderCommandBuffers[i].end();
	}
}

void VkSampleRenderer::RecreateSwapChain()
{
	// WIP: use old swap chain during recreation
	// vkDeviceWaitIdle(m_device);

	CleanupSwapChain();

	// CreateSwapChain();
	// CreateSwapChainImageViews();
	// CreateRenderPasses();
	// CreateGraphicsPipeline();
	// CreateFramebuffers();
	// CreateUniformBuffers();
	// CreateRenderCommandBuffers();
}

void VkSampleRenderer::CleanupSwapChain()
{
	// for (auto framebuffer : m_swapChainFramebuffers) {
	//	vkDestroyFramebuffer(m_device, framebuffer, nullptr);
	//}

	// WIP:
	// vkFreeCommandBuffers(
	//	m_device, m_graphicsCommandPool, static_cast<uint32>(m_commandBuffers.size()), m_commandBuffers.data());

	// vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
	// vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	// vkDestroyRenderPass(m_device, m_renderPass, nullptr);

	// for (auto imageView : m_swapChainImageViews) {
	//	vkDestroyImageView(m_device, imageView, nullptr);
	//}

	// vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);

	// for (size_t i = 0; i < m_swapchain->GetImageCount(); i++) {
	//	vkDestroyBuffer(m_device, m_uniformBuffers[i], nullptr);
	//	vkFreeMemory(m_device, m_uniformBuffersMemory[i], nullptr);
	//}

	// vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
}

void VkSampleRenderer::Init(HWND assochWnd, HINSTANCE instance)
{
	m_instanceLayer = std::make_unique<vlkn::InstanceLayer>(assochWnd, instance);

	auto physicalDevice = m_instanceLayer->GetBestCapablePhysicalDevice();

	m_device = physicalDevice->RequestLogicalDevice();

	m_swapchain = m_device->RequestDeviceSwapchainOnSurface(m_instanceLayer->GetSurface());

	m_graphicsPipeline = m_device->RequestDeviceGraphicsPipeline(m_swapchain.get());

	m_descriptors = m_device->RequestDeviceDescriptors(m_swapchain.get(), m_graphicsPipeline.get());

	CreateRenderCommandBuffers();

	// semaphores

	m_imageAvailableSemaphore = m_device->createSemaphoreUnique({});
	m_renderFinishedSemaphore = m_device->createSemaphoreUnique({});
} // namespace vk

bool VkSampleRenderer::SupportsEditor()
{
	return false;
}

void VkSampleRenderer::DrawFrame()
{
	uint32 imageIndex;

	vk::Result result = m_device->acquireNextImageKHR(
		m_swapchain->Get(), UINT64_MAX, m_imageAvailableSemaphore.get(), {}, &imageIndex);

	switch (result) {
		case vk::Result::eErrorOutOfDateKHR: RecreateSwapChain(); return;
		case vk::Result::eSuccess:
		case vk::Result::eSuboptimalKHR: break;
		default: LOG_ABORT("failed to acquire swap chain image!");
	}

	// WIP: UPDATES
	{
		auto world = Engine::GetWorld();
		auto geomNode = world->GetAnyAvailableNode<GeometryNode>();
		auto modelm = geomNode->GetNodeTransformWCS();
		auto camera = world->GetActiveCamera();

		vlkn::UniformBufferObject ubo = {};
		ubo.model = modelm;
		ubo.view = camera->GetViewMatrix();
		ubo.proj = camera->GetProjectionMatrix();

		void* data = m_device->mapMemory(m_descriptors->GetUniformBuffersMemory()[imageIndex], 0, sizeof(ubo));
		memcpy(data, &ubo, sizeof(ubo));
		m_device->unmapMemory(m_descriptors->GetUniformBuffersMemory()[imageIndex]);
	}

	vk::SubmitInfo submitInfo{};
	vk::Semaphore waitSemaphores[] = { m_imageAvailableSemaphore.get() };

	// wait with writing colors to the image until it's available
	// the implementation can already start executing our vertex shader and such while the image is not yet available
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo.setWaitSemaphoreCount(1u)
		.setPWaitSemaphores(waitSemaphores)
		.setPWaitDstStageMask(waitStages)
		.setCommandBufferCount(1u)
		.setPCommandBuffers(&m_renderCommandBuffers[imageIndex]);

	// which semaphores to signal once the command buffer(s) have finished execution
	vk::Semaphore signalSemaphores[] = { m_renderFinishedSemaphore.get() };
	submitInfo.setSignalSemaphoreCount(1u).setPSignalSemaphores(signalSemaphores);

	m_device->SubmitGraphics(1u, &submitInfo, {});

	vk::PresentInfoKHR presentInfo;
	presentInfo.setWaitSemaphoreCount(1u).setPWaitSemaphores(signalSemaphores);

	vk::SwapchainKHR swapChains[] = { m_swapchain->Get() };
	presentInfo.setSwapchainCount(1u).setPSwapchains(swapChains).setPImageIndices(&imageIndex).setPResults(nullptr);

	result = m_device->Present(presentInfo);

	switch (result) {
		case vk::Result::eErrorOutOfDateKHR:
		case vk::Result::eSuboptimalKHR: RecreateSwapChain(); return;
		case vk::Result::eSuccess: break;
		default: LOG_ABORT("failed to acquire swap chain image!");
	}
}

} // namespace vlkn
