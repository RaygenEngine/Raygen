#pragma once
#include "engine/Listener.h"
#include "rendering/out/CopyHdrTexture.h"
#include "rendering/ppt/PtCollection.h"
#include "rendering/scene/Scene.h"
#include "rendering/wrappers/RGbuffer.h"

namespace vl {



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

	UniquePtr<RGbuffer> m_gbuffer;

	
	void RecordGeometryPasses(vk::CommandBuffer* cmdBuffer, SceneRenderDesc& sceneDesc);
	void RecordPostProcessPass(vk::CommandBuffer* cmdBuffer, SceneRenderDesc& sceneDesc);
	void RecordOutPass(vk::CommandBuffer* cmdBuffer, SceneRenderDesc& sceneDesc, vk::RenderPass outRp,
		vk::Framebuffer outFb, vk::Extent2D outExtent);

	PtCollection m_postprocCollection;

protected:
	// CHECK: boolflag event, (impossible to use here current because of init order)
	BoolFlag m_didViewportResize;
	BoolFlag m_didWindowResize;

	void OnViewportResize();

	bool m_isMinimzed{ false };

public:
	// TODO: POSTPROC post process for hdr, move those
	FrameArray<vk::UniqueFramebuffer> m_framebuffers;
	FrameArray<UniquePtr<RImageAttachment>> m_attachments;
	FrameArray<UniquePtr<RImageAttachment>> m_attachments2;

	// std::array<UniquePtr<RImageAttachment>, 3> m_attachmentsDepthToUnlit;

	FrameArray<vk::DescriptorSet> m_ppDescSets;
	vk::UniqueRenderPass m_ptRenderpass;


	// TODO: RT, move those
	vk::UniqueAccelerationStructureKHR sceneAS;

	Renderer_();
	~Renderer_();

	void UpdateForFrame();

	void DrawFrame(vk::CommandBuffer* cmdBuffer, SceneRenderDesc& sceneDesc, vk::RenderPass outRp,
		vk::Framebuffer outFb, vk::Extent2D outExtent);

	void InitPipelines(vk::RenderPass outRp);



	[[nodiscard]] vk::Viewport GetSceneViewport() const;
	[[nodiscard]] vk::Viewport GetGameViewport() const;

	[[nodiscard]] vk::Rect2D GetSceneScissor() const;
	[[nodiscard]] vk::Rect2D GetGameScissor() const;

	[[nodiscard]] RGbuffer* GetGbuffer() const { return m_gbuffer.get(); }
} * Renderer{};
} // namespace vl
