#pragma once
#include "engine/Events.h"
#include "platform/GlfwUtl.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/DeferredPass.h"
#include "rendering/EditorPass.h"
#include "rendering/GeometryPass.h"
#include "rendering/objects/Buffer.h"
#include "rendering/objects/PhysicalDevice.h"
#include "rendering/renderer/Swapchain.h"
#include "rendering/resource/GpuResources.h"

#include <vulkan/vulkan.hpp>

namespace vl {
using SemVec = std::vector<vk::Semaphore>;

inline class Renderer_ : public Listener {

	//
	// Framebuffer size, Window size, recommended framebuffer size and more
	//

	// The recommended framebuffer allocation size for the viewport.
	vk::Extent2D m_viewportFramebufferSize{};

	// The actual game viewport rectangle in m_swapchain coords
	vk::Rect2D m_viewportRect{};

	UniquePtr<GeometryPass> m_geomPass;
	UniquePtr<DeferredPass> m_defPass;
	UniquePtr<EditorPass> m_editorPass;

	UniquePtr<GBuffer> m_gBuffer;
	UniquePtr<Swapchain> m_swapchain;

	std::vector<vk::CommandBuffer> m_geometryCmdBuffer;
	std::vector<vk::CommandBuffer> m_outCmdBuffer;

	std::vector<vk::UniqueFence> m_inFlightFence;

	std::vector<vk::UniqueSemaphore> m_renderFinishedSem;
	std::vector<vk::UniqueSemaphore> m_imageAvailSem;

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

	void DrawGeometryPass(vk::CommandBuffer cmdBuffer);

	void UpdateForFrame();

	void DrawDeferredPass(vk::CommandBuffer cmdBuffer, vk::Framebuffer framebuffer);

	void DrawFrame();

	[[nodiscard]] vk::Extent2D GetViewportFramebufferSize() { return m_viewportFramebufferSize; }
	[[nodiscard]] vk::Rect2D GetViewportRect() { return m_viewportRect; }

	[[nodiscard]] GBuffer* GetGBuffer() const { return m_gBuffer.get(); }
	[[nodiscard]] Swapchain* GetSwapchain() const { return m_swapchain.get(); }

	[[nodiscard]] GeometryPass* GetGeometryPass() const { return m_geomPass.get(); }
	[[nodiscard]] DeferredPass* GetDeferredPass() const { return m_defPass.get(); }
	[[nodiscard]] EditorPass* GetEditorPass() const { return m_editorPass.get(); }

	inline static uint32 currentFrame{ 0 };

} * Renderer{};
} // namespace vl
