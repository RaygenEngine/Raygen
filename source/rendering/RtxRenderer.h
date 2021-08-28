#pragma once

#include "rendering/ppt/techniques/PtLightBlend.h"
#include "rendering/RendererBase.h"
#include "rendering/wrappers/CmdBuffer.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"
#include "rendering/techniques/TestSVGFProgPT.h"

namespace vl {

inline class RtxRenderer_ : public RendererBase {

public:
	RtxRenderer_();

	void ResizeBuffers(uint32 width, uint32 height) override;

	void RecordCmd(vk::CommandBuffer cmdBuffer, SceneRenderDesc&& sceneDesc, OutputPassBase& outputPass) override;

	InFlightResources<vk::ImageView> GetOutputViews() const override;


private:
	InFlightResources<RenderingPassInstance> m_mainPassInst;

	TestSVGFProgPT m_testTech;

	vk::Extent2D m_extent{};

	void* m_viewerPtr{ nullptr };
	InFlightResources<vk::DescriptorSet> m_globalDesc;

	void UpdateGlobalDescSet(SceneRenderDesc& sceneDesc);
	void DrawGeometryAndLights(vk::CommandBuffer cmdBuffer, SceneRenderDesc& sceneDesc);

} * RtxRenderer{};
} // namespace vl
