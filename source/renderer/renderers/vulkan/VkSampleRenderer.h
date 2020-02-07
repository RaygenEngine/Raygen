#pragma once
#include "renderer/renderers/vulkan/InstanceLayer.h"
#include "renderer/renderers/vulkan/Device.h"
#include "renderer/renderers/vulkan/Swapchain.h"
#include "renderer/renderers/vulkan/GraphicsPipeline.h"
#include "renderer/renderers/vulkan/Descriptors.h"
#include "renderer/renderers/vulkan/Model.h"
#include "renderer/Renderer.h"

#include "system/EngineEvents.h"

#include <vulkan/vulkan.hpp>


namespace vlkn {

class VkSampleRenderer : public Renderer {
	friend class ImguiImpl;

	// high level parts
	std::unique_ptr<InstanceLayer> m_instanceLayer;
	std::unique_ptr<Device> m_device;
	std::unique_ptr<Swapchain> m_swapchain;
	std::unique_ptr<GraphicsPipeline> m_graphicsPipeline;
	std::unique_ptr<Descriptors> m_descriptors;

	// data
	std::vector<std::unique_ptr<Model>> m_models;

	// render commands
	std::vector<vk::CommandBuffer> m_renderCommandBuffers;

	// sync objects
	vk::UniqueSemaphore m_imageAvailableSemaphore;
	vk::UniqueSemaphore m_renderFinishedSemaphore;

	// WIP : handle resizing explicitly in case there are drivers
	// that do not report swap chain incompatibilities correctly
	// Also test window minimization
	void AllocateRenderCommandBuffers();

	DECLARE_EVENT_LISTENER(m_resizeListener, Event::OnWindowResize);
	DECLARE_EVENT_LISTENER(m_worldLoaded, Event::OnWorldLoaded);
	bool m_shouldRecreateSwapchain{ false };

	void RecordCommandBuffer(int32 imageIndex);

public:
	virtual ~VkSampleRenderer();

	void InitInstanceAndSurface(std::vector<const char*> additionalExtensions, WindowType* window);

	void InitWorld();

	void Init();
	virtual bool SupportsEditor() override;
	virtual void DrawFrame() override;
};

} // namespace vlkn
