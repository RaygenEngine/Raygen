#pragma once
#include "renderer/renderers/vulkan/InstanceLayer.h"
#include "renderer/renderers/vulkan/Device.h"
#include "renderer/renderers/vulkan/Swapchain.h"
#include "renderer/renderers/vulkan/GraphicsPipeline.h"
#include "renderer/renderers/vulkan/Descriptors.h"
#include "renderer/renderers/vulkan/Model.h"
#include "renderer/ObserverRenderer.h"

#include <vulkan/vulkan.hpp>


namespace vk {

class VkRendererBase : public ObserverRenderer {

	// high level parts
	std::unique_ptr<vulkan::InstanceLayer> m_instanceLayer;
	std::unique_ptr<vulkan::Device> m_device;
	std::unique_ptr<vulkan::Swapchain> m_swapchain;
	std::unique_ptr<vulkan::GraphicsPipeline> m_graphicsPipeline;
	std::unique_ptr<vulkan::Descriptors> m_descriptors;

	// data
	std::unique_ptr<vulkan::Model> m_model;

	// render commands
	std::vector<vk::CommandBuffer> m_renderCommandBuffers;

	// sync objects
	vk::UniqueSemaphore m_imageAvailableSemaphore;
	vk::UniqueSemaphore m_renderFinishedSemaphore;

	// WIP : handle resizing explicitly in case there are drivers
	// that do not report swap chain incompatibilities correctly
	// Also test window minimization
	void CreateRenderCommandBuffers();

	void RecreateSwapChain();
	void CleanupSwapChain();

public:
	virtual ~VkRendererBase();


	virtual void Init(HWND assochWnd, HINSTANCE instance) override;
	virtual bool SupportsEditor() override;
	virtual void Render() override;
};

} // namespace vk
