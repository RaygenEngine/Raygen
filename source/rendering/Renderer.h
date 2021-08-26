#pragma once

#include "rendering/ppt/techniques/PtLightBlend.h"
#include "rendering/RendererBase.h"
#include "rendering/techniques/RaytraceArealights.h"
#include "rendering/techniques/RaytraceMirrorReflections.h"
#include "rendering/wrappers/CmdBuffer.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

namespace vl {

inline class Renderer_ : public RendererBase {

public:
	Renderer_();

	void ResizeBuffers(uint32 width, uint32 height) override;

	void DrawFrame(vk::CommandBuffer cmdBuffer, SceneRenderDesc&& sceneDesc, OutputPassBase& outputPass) override;

	InFlightResources<vk::ImageView> GetOutputViews() const override;

private:
	// TODO: ppt
	PtLightBlend m_ptLightBlend;
	InFlightResources<RenderingPassInstance> m_ptPass;
	//

	// TODO: geometry + core light
	InFlightResources<RenderingPassInstance> m_mainPassInst;
	InFlightResources<RenderingPassInstance> m_secondaryPassInst;
	InFlightResources<RenderingPassInstance> m_unlitPassInst;
	//

	// non-static techniques
	RaytraceMirrorReflections m_raytraceMirrorReflections;
	RaytraceArealights m_raytraceArealights;

	vk::Extent2D m_extent{};

	void* m_viewerPtr{ nullptr };
	InFlightResources<vk::DescriptorSet> m_globalDesc;

	void UpdateGlobalDescSet(SceneRenderDesc& sceneDesc);
	void DrawGeometryAndLights(vk::CommandBuffer cmdBuffer, SceneRenderDesc& sceneDesc);

} * Renderer{};
} // namespace vl
