#pragma once
#include "renderer/InstanceLayer.h"
#include "renderer/Device.h"
#include "renderer/Swapchain.h"
#include "renderer/GraphicsPipeline.h"
#include "renderer/Descriptors.h"
#include "renderer/Model.h"

#include "system/EngineEvents.h"

#include <vulkan/vulkan.hpp>


namespace vlkn {

class VkSampleRenderer {
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
	DECLARE_EVENT_LISTENER(m_viewportUpdated, Event::OnViewportUpdated);
	bool m_shouldRecreateSwapchain{ false };
	bool m_shouldRecreatePipeline{ false };


	void RecordCommandBuffer(int32 imageIndex);

public:
	virtual ~VkSampleRenderer();

	void InitInstanceAndSurface(std::vector<const char*> additionalExtensions, WindowType* window);

	void InitWorld();

	void RecreateGraphicsPipeline();

	void Init();
	void DrawFrame();
};

} // namespace vlkn
