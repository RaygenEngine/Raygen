#include "pch/pch.h"

#include "renderer/VkSampleRenderer.h"
#include "system/Logger.h"
#include "system/Engine.h"

#include "asset/AssetManager.h"
#include "world/World.h"
#include "world/nodes/geometry/GeometryNode.h"
#include "world/nodes/camera/CameraNode.h"
#include "editor/imgui/ImguiImpl.h"
#include "system/profiler/ProfileScope.h"

#include <set>


VkSampleRenderer::~VkSampleRenderer()
{
	m_device->waitIdle();

	Engine::Get().m_remakeWindow = true;
}

void VkSampleRenderer::CreateGeometry()
{
	auto world = Engine::GetWorld();

	for (auto geomNode : world->GetNodeIterator<GeometryNode>()) {

		auto model = geomNode->GetModel();
		m_models.emplace_back(std::make_unique<Model>(m_device, m_descriptors.get(), model));
		m_models.back()->m_transform = geomNode->GetNodeTransformWCS();
	}
}

void VkSampleRenderer::AllocateRenderCommandBuffers()
{
	m_renderCommandBuffers.clear();

	m_renderCommandBuffers.resize(m_swapchain->GetImageCount());

	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setCommandPool(m_device.GetGraphicsCommandPool())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(static_cast<uint32>(m_renderCommandBuffers.size()));

	m_renderCommandBuffers = m_device->allocateCommandBuffers(allocInfo);
}

void VkSampleRenderer::InitInstanceAndSurface(std::vector<const char*> additionalExtensions, WindowType* window)
{
	// WIP: this is not renderer code ....
	m_instance.Init(additionalExtensions, window);
	auto& physicalDevice = m_instance.GetBestCapablePhysicalDevice();
	m_device.Init(physicalDevice, { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME });
	// ......//
}

void VkSampleRenderer::InitWorld()
{
	CreateGeometry();
}

void VkSampleRenderer::RecreateGraphicsPipeline()
{
}

void VkSampleRenderer::Init()
{
	m_resizeListener.Bind([&](auto, auto) { m_shouldRecreateSwapchain = true; });
	m_worldLoaded.Bind([&]() { InitWorld(); });
	m_viewportUpdated.Bind([&]() { m_shouldRecreatePipeline = true; });

	m_swapchain = m_device.RequestDeviceSwapchainOnSurface(m_instance.GetSurface());
	m_graphicsPipeline = m_device.RequestDeviceGraphicsPipeline(m_swapchain.get());
	m_descriptors = m_device.RequestDeviceDescriptors(m_swapchain.get(), m_graphicsPipeline.get());

	AllocateRenderCommandBuffers();

	// semaphores
	m_imageAvailableSemaphore = m_device->createSemaphoreUnique({});
	m_renderFinishedSemaphore = m_device->createSemaphoreUnique({});

	::ImguiImpl::InitVulkan();
} // namespace vk

void VkSampleRenderer::RecordCommandBuffer(int32 imageIndex)
{
	PROFILE_SCOPE(Renderer);

	auto& cmdBuffer = m_renderCommandBuffers[imageIndex];
	// WIP

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlags(0)).setPInheritanceInfo(nullptr);

	// begin command buffer recording
	cmdBuffer.begin(beginInfo);
	{
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo.setRenderPass(m_swapchain->GetRenderPass())
			.setFramebuffer(m_swapchain->GetFramebuffers()[imageIndex]);
		renderPassInfo.renderArea.setOffset({ 0, 0 }).setExtent(m_swapchain->GetExtent());
		std::array<vk::ClearValue, 2> clearValues = {};
		clearValues[0].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
		clearValues[1].setDepthStencil({ 1.0f, 0 });
		renderPassInfo.setClearValueCount(static_cast<uint32>(clearValues.size()));
		renderPassInfo.setPClearValues(clearValues.data());

		// begin render pass
		cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
		{
			// bind the graphics pipeline
			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline->GetPipeline());


			for (auto& model : m_models) {
				for (auto& gg : model->GetGeometryGroups()) {
					PROFILE_SCOPE(Renderer);

					// Submit via push constant (rather than a UBO)
					cmdBuffer.pushConstants(m_graphicsPipeline->GetPipelineLayout(), vk::ShaderStageFlagBits::eVertex,
						0u, sizeof(glm::mat4), &model->m_transform);

					vk::Buffer vertexBuffers[] = { gg.vertexBuffer.get() };
					vk::DeviceSize offsets[] = { 0 };
					// geom
					cmdBuffer.bindVertexBuffers(0u, 1u, vertexBuffers, offsets);

					// indices
					cmdBuffer.bindIndexBuffer(gg.indexBuffer.get(), 0, vk::IndexType::eUint32);

					// descriptor sets
					cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
						m_graphicsPipeline->GetPipelineLayout(), 0u, 1u, &(gg.descriptorSets[imageIndex]), 0u, nullptr);

					// draw call (triangle)
					cmdBuffer.drawIndexed(gg.indexCount, 1u, 0u, 0u, 0u);
				}
			}

			::ImguiImpl::RenderVulkan(&cmdBuffer);
		}
		// end render pass
		cmdBuffer.endRenderPass();
	}
	// end command buffer recording
	cmdBuffer.end();
}

void VkSampleRenderer::DrawFrame()
{
	PROFILE_SCOPE(Renderer);

	if (m_shouldRecreateSwapchain) {
		m_device->waitIdle();

		m_descriptors.reset();
		m_graphicsPipeline.reset();
		m_swapchain.reset();

		m_device->freeCommandBuffers(m_device.GetGraphicsCommandPool(), m_renderCommandBuffers);

		m_swapchain = m_device.RequestDeviceSwapchainOnSurface(m_instance.GetSurface());
		m_graphicsPipeline = m_device.RequestDeviceGraphicsPipeline(m_swapchain.get());
		m_descriptors = m_device.RequestDeviceDescriptors(m_swapchain.get(), m_graphicsPipeline.get());

		m_models.clear();
		CreateGeometry();

		m_renderCommandBuffers.clear();
		AllocateRenderCommandBuffers();

		m_shouldRecreateSwapchain = false;
		m_shouldRecreatePipeline = false;
		::ImguiImpl::InitVulkan();
	}
	else if (m_shouldRecreatePipeline) {
		m_shouldRecreatePipeline = false;

		m_descriptors.reset();
		m_graphicsPipeline.reset();


		m_graphicsPipeline = m_device.RequestDeviceGraphicsPipeline(m_swapchain.get());
		m_descriptors = m_device.RequestDeviceDescriptors(m_swapchain.get(), m_graphicsPipeline.get());

		m_models.clear();
		CreateGeometry();

		::ImguiImpl::InitVulkan();
	}

	uint32 imageIndex;

	vk::Result result0 = m_device->acquireNextImageKHR(
		m_swapchain->Get(), UINT64_MAX, m_imageAvailableSemaphore.get(), {}, &imageIndex);

	switch (result0) {
		case vk::Result::eErrorOutOfDateKHR: return;
		case vk::Result::eSuccess:
		case vk::Result::eSuboptimalKHR: break;
		default: LOG_ABORT("failed to acquire swap chain image!");
	}

	// WIP: UNIFORM BUFFER UPDATES
	{
		auto world = Engine::GetWorld();
		auto camera = world->GetActiveCamera();

		UniformBufferObject ubo{};
		ubo.view = camera->GetViewMatrix();
		ubo.proj = camera->GetProjectionMatrix();

		void* data = m_device->mapMemory(m_descriptors->GetUniformBuffersMemory()[imageIndex], 0, sizeof(ubo));
		memcpy(data, &ubo, sizeof(ubo));
		m_device->unmapMemory(m_descriptors->GetUniformBuffersMemory()[imageIndex]);
	}

	vk::SubmitInfo submitInfo{};
	vk::Semaphore waitSemaphores[] = { m_imageAvailableSemaphore.get() };


	RecordCommandBuffer(imageIndex);

	// wait with writing colors to the image until it's available
	// the implementation can already start executing our vertex shader and such while the image is not yet
	// available
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	submitInfo.setWaitSemaphoreCount(1u)
		.setPWaitSemaphores(waitSemaphores)
		.setPWaitDstStageMask(waitStages)
		.setCommandBufferCount(1u)
		.setPCommandBuffers(&m_renderCommandBuffers[imageIndex]);

	// which semaphores to signal once the command buffer(s) have finished execution
	vk::Semaphore signalSemaphores[] = { m_renderFinishedSemaphore.get() };
	submitInfo.setSignalSemaphoreCount(1u).setPSignalSemaphores(signalSemaphores);

	m_device.GetGraphicsQueue()->submit(1u, &submitInfo, {});
	vk::PresentInfoKHR presentInfo;
	presentInfo.setWaitSemaphoreCount(1u).setPWaitSemaphores(signalSemaphores);

	vk::SwapchainKHR swapChains[] = { m_swapchain->Get() };
	presentInfo.setSwapchainCount(1u).setPSwapchains(swapChains).setPImageIndices(&imageIndex).setPResults(nullptr);

	vk::Result result1 = m_device.GetPresentQueue()->presentKHR(presentInfo);

	m_device.GetPresentQueue()->waitIdle();

	switch (result1) {
		case vk::Result::eErrorOutOfDateKHR:
		case vk::Result::eSuboptimalKHR: return;
		case vk::Result::eSuccess: break;
		default: LOG_ABORT("failed to acquire swap chain image!");
	}
}
