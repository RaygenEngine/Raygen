#pragma once
#include "engine/Listener.h"
#include "rendering/out/CopyHdrTexture.h"
#include "rendering/ppt/PtCollection.h"
#include "rendering/scene/Scene.h"
#include "rendering/wrappers/RGbuffer.h"

namespace vl {


inline class Renderer_ : public Listener {
	// The recommended framebuffer allocation size for the viewport.
	vk::Extent2D m_viewportFramebufferSize{};

	// The actual game viewport rectangle in m_swapchain coords
	vk::Rect2D m_viewportRect{};

private:
	CopyHdrTexture m_copyHdrTexture;

	FrameArray<RGbuffer> m_gbuffer;


	void RecordGeometryPasses(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordPostProcessPass(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc);
	void RecordOutPass(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc, vk::RenderPass outRp,
		vk::Framebuffer outFb, vk::Extent2D outExtent);

	PtCollection m_postprocCollection;


	struct SecondaryBufferPool {

		std::vector<FrameArray<vk::CommandBuffer>> sBuffers;

		vk::CommandBuffer Get(uint32 frameIndex)
		{
			if (currBuffer > (int32(sBuffers.size()) - 1)) {

				vk::CommandBufferAllocateInfo allocInfo{};
				allocInfo.setCommandPool(Device->mainCmdPool.get())
					.setLevel(vk::CommandBufferLevel::eSecondary)
					.setCommandBufferCount(c_framesInFlight);

				// allocate all buffers needed
				{
					auto buffers = Device->allocateCommandBuffers(allocInfo);

					auto moveBuffersToArray = [&buffers](auto& target, size_t index) {
						auto begin = buffers.begin() + (index * c_framesInFlight);
						std::move(begin, begin + c_framesInFlight, target.begin());
					};

					sBuffers.push_back({});
					moveBuffersToArray(sBuffers[currBuffer], 0);
				}
			}

			return sBuffers[currBuffer++][frameIndex];
		}

		void Top() { currBuffer = 0; }

		int32 currBuffer{ 0 };

	} m_secondaryBuffersPool;

protected:
	// CHECK: boolflag event, (impossible to use here current because of init order)
	BoolFlag m_didViewportResize;

	void OnViewportResize();

public:
	// TODO: POSTPROC post process for hdr, move those
	FrameArray<vk::UniqueFramebuffer> m_framebuffer;
	FrameArray<RImageAttachment> m_attachment;
	FrameArray<RImageAttachment> m_attachment2;

	// std::array<UniquePtr<RImageAttachment>, 3> m_attachmentsDepthToUnlit;

	FrameArray<vk::DescriptorSet> m_ppDescSet;
	vk::UniqueRenderPass m_ptRenderpass;


	// TODO: RT, move those
	vk::UniqueAccelerationStructureKHR sceneAS;

	Renderer_();

	void PrepareForFrame();

	void DrawFrame(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc, vk::RenderPass outRp,
		vk::Framebuffer outFb, vk::Extent2D outExtent);

	void InitPipelines(vk::RenderPass outRp);

	[[nodiscard]] RGbuffer& GetGbuffer() { return m_gbuffer.at(0); }
} * Renderer{};
} // namespace vl
