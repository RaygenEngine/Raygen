#pragma once

#include "engine/Listener.h"
#include "rendering/ppt/techniques/PtLightBlend.h"
#include "rendering/techniques/RaytraceArealights.h"
#include "rendering/techniques/RaytraceMirrorReflections.h"
#include "rendering/wrappers/CmdBuffer.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

namespace vl {
class OutputPassBase;
}
struct SceneRenderDesc;

namespace vl {
// WIP:
class RendererBase : public Listener {
public:
	virtual void ResizeBuffers(uint32 width, uint32 height) = 0;
	virtual void DrawFrame(vk::CommandBuffer cmdBuffer, SceneRenderDesc&& sceneDesc, OutputPassBase& outputPass) = 0;
	virtual InFlightResources<vk::ImageView> GetOutputViews() const = 0;

	virtual ~RendererBase() = default;
};

inline class Renderer_ : public RendererBase {

public:
	Renderer_();

	void InitPipelines();

	void ResizeBuffers(uint32 width, uint32 height) override;

	void DrawFrame(vk::CommandBuffer cmdBuffer, SceneRenderDesc&& sceneDesc, OutputPassBase& outputPass) override;

	InFlightResources<vk::ImageView> GetOutputViews() const override;


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

private:
	vk::Extent2D m_extent{};

	InFlightResources<size_t> m_viewerId{ ~0llu };
	InFlightResources<vk::DescriptorSet> m_globalDesc;

	void UpdateGlobalDescSet(SceneRenderDesc& sceneDesc);
	void DrawGeometryAndLights(vk::CommandBuffer cmdBuffer, SceneRenderDesc& sceneDesc);

} * Renderer{};
} // namespace vl
