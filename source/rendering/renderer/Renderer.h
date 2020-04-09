#pragma once
#include "engine/Events.h"
#include "platform/GlfwUtl.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/DeferredPass.h"
#include "rendering/EditorPass.h"
#include "rendering/GeometryPass.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/objects/Buffer.h"
#include "rendering/objects/PhysicalDevice.h"
#include "rendering/renderer/Swapchain.h"
#include "rendering/scene/SceneStructs.h"
#include "rendering/scene/Scene.h"


#include <vulkan/vulkan.hpp>

// WIP: position, direction, etc
struct UBO_Camera {
	glm::mat4 viewProj;
};

namespace vl {
using SemVec = std::vector<vk::Semaphore>;

inline class Renderer_ : public Listener {
public:
	//
	// Framebuffer size, Window size, recommended framebuffer size and more
	//

	// The recommended framebuffer allocation size for the viewport.
	vk::Extent2D m_viewportFramebufferSize{};

	// The actual game viewport rectangle in m_swapchain coords
	vk::Rect2D m_viewportRect{};

	// Camera desc
	DescriptorLayout m_cameraDescLayout;
	std::vector<vk::DescriptorSet> m_camDescSet;
	std::vector<UniquePtr<Buffer<UBO_Camera>>> m_cameraUBO;

protected:
	// CHECK: boolflag event, (impossible to use here current because of init order)
	BoolFlag m_didViewportResize;
	BoolFlag m_didWindowResize;

	void OnViewportResize();
	void OnWindowResize();

	bool m_isMinimzed{ false };

public:
	Renderer_();
	~Renderer_();


	void Init();
	void ReconstructSwapchain();

	GeometryPass m_geomPass;
	DeferredPass m_defPass;
	EditorPass m_editorPass;


	Swapchain* m_swapchain{};

	//
	std::vector<vk::CommandBuffer> m_geometryCmdBuffer;
	std::vector<vk::CommandBuffer> m_outCmdBuffer;


	std::vector<vk::UniqueFence> m_inFlightFence;

	std::vector<vk::UniqueSemaphore> m_renderFinishedSem;
	std::vector<vk::UniqueSemaphore> m_imageAvailSem;

	uint32 m_currentFrame{ 0 };

	void DrawGeometryPass(vk::CommandBuffer cmdBuffer);

	void UpdateForFrame();

	void DrawDeferredPass(vk::CommandBuffer cmdBuffer, vk::Framebuffer framebuffer);

	vk::DescriptorSetLayout GetCameraDescLayout() { return m_cameraDescLayout.setLayout.get(); }
	vk::DescriptorSet GetCameraDescSet() { return m_camDescSet[m_currentFrame]; }

	void UpdateCamera();

	void DrawFrame();
} * Renderer{};
} // namespace vl
