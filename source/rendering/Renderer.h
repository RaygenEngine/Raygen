#pragma once
#include "rendering/wrappers/RGbuffer.h"
#include "rendering/out/CopyHdrTexture.h"
#include "rendering/out/WriteEditor.h"
#include "rendering/passes/DepthmapPass.h"
#include "rendering/ppt/PtCollection.h"

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

private:
	CopyHdrTexture m_copyHdrTexture;
	WriteEditor m_writeEditor;

	UniquePtr<RGbuffer> m_gbuffer;

	std::vector<vk::CommandBuffer> m_geometryCmdBuffer;
	std::vector<vk::CommandBuffer> m_pptCmdBuffer;
	std::vector<vk::CommandBuffer> m_outCmdBuffer;

	std::vector<vk::UniqueFence> m_inFlightFence;

	std::vector<vk::UniqueSemaphore> m_renderFinishedSem;
	std::vector<vk::UniqueSemaphore> m_imageAvailSem;

	void RecordGeometryPasses(vk::CommandBuffer* cmdBuffer);
	void RecordPostProcessPass(vk::CommandBuffer* cmdBuffer);
	void RecordOutPass(vk::CommandBuffer* cmdBuffer);

	PtCollection m_postprocCollection;

protected:
	// CHECK: boolflag event, (impossible to use here current because of init order)
	BoolFlag m_didViewportResize;
	BoolFlag m_didWindowResize;

	void OnViewportResize();
	void OnWindowResize();

	bool m_isMinimzed{ false };

public:
	// post process for hdr WIP: move those
	std::array<vk::UniqueFramebuffer, 3> m_framebuffers;
	std::array<UniquePtr<RImageAttachment>, 3> m_attachments;
	std::array<UniquePtr<RImageAttachment>, 3> m_attachments2;
	std::array<vk::DescriptorSet, 3> m_ppDescSets;
	vk::UniqueRenderPass m_ptRenderpass;

	Renderer_();
	~Renderer_();

	void UpdateForFrame();

	void DrawFrame();
	void InitPipelines();

	inline static uint32 currentFrame{ 0 };

	[[nodiscard]] vk::Viewport GetSceneViewport() const;
	[[nodiscard]] vk::Viewport GetGameViewport() const;

	[[nodiscard]] vk::Rect2D GetSceneScissor() const;
	[[nodiscard]] vk::Rect2D GetGameScissor() const;

	[[nodiscard]] RGbuffer* GetGbuffer() const { return m_gbuffer.get(); }
} * Renderer{};
} // namespace vl
