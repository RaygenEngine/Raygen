#pragma once
#include "engine/Events.h"
#include "platform/GlfwUtl.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/objects/RBuffer.h"
#include "rendering/objects/RPhysicalDevice.h"
#include "rendering/passes/EditorPass.h"
#include "rendering/passes/GBufferPass.h"
#include "rendering/passes/ShadowmapPass.h"
#include "rendering/passes/SpotlightPass.h"
#include "rendering/passes/AmbientPass.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/Swapchain.h"
#include "rendering/ppt/PtCollection.h"

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

	GBufferPass m_gBufferPass;
	ShadowmapPass m_shadowmapPass;
	SpotlightPass m_spotlightPass;
	AmbientPass m_ambientPass;
	EditorPass m_editorPass;

	UniquePtr<GBuffer> m_gBuffer;

	std::vector<vk::CommandBuffer> m_geometryCmdBuffer;
	std::vector<vk::CommandBuffer> m_outCmdBuffer;

	std::vector<vk::UniqueFence> m_inFlightFence;

	std::vector<vk::UniqueSemaphore> m_renderFinishedSem;
	std::vector<vk::UniqueSemaphore> m_imageAvailSem;

	void RecordGeometryPasses(vk::CommandBuffer* cmdBuffer);

	void RecordDeferredPasses(vk::CommandBuffer* cmdBuffer);


	PtCollection m_postprocCollection;

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

	void UpdateForFrame();

	void DrawFrame();

	inline static uint32 currentFrame{ 0 };

	[[nodiscard]] vk::Viewport GetSceneViewport() const;
	[[nodiscard]] vk::Viewport GetViewport() const;

	[[nodiscard]] vk::Rect2D GetSceneScissor() const;
	[[nodiscard]] vk::Rect2D GetScissor() const;

	[[nodiscard]] GBuffer* GetGBuffer() const { return m_gBuffer.get(); }

	// TODO: remove
	vk::RenderPass GetShadowmapRenderPass() const { return m_shadowmapPass.GetRenderPass(); }

} * Renderer{};
} // namespace vl
