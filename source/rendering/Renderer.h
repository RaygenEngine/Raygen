#pragma once

#include "engine/Listener.h"
#include "rendering/ppt/techniques/PtLightBlend.h"
#include "rendering/techniques/RaytraceMirrorReflections.h"
#include "rendering/wrappers/CmdBuffer.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

namespace vl {
class OutputPassBase;
}
struct SceneRenderDesc;

namespace vl {

inline class Renderer_ : public Listener {

public:
	void InitPipelines();

	void ResizeBuffers(uint32 width, uint32 height);

	void DrawFrame(vk::CommandBuffer cmdBuffer, SceneRenderDesc& sceneDesc, OutputPassBase& outputPass);

	InFlightResources<vk::ImageView> GetOutputViews() const;


	// TODO: ppt
	PtLightBlend m_ptLightBlend;
	InFlightResources<RenderingPassInstance> m_ptPass;
	//

	// TODO: geometry + core light
	InFlightResources<RenderingPassInstance> m_mainPassInst;
	InFlightResources<RenderingPassInstance> m_secondaryPassInst;
	//

	// non-static techniques
	RaytraceMirrorReflections m_raytraceMirrorReflections;

private:
	vk::Extent2D m_extent{};

	// TODO: Global descriptor set
	InFlightResources<vk::DescriptorSet> m_attachmentsDesc;

	void DrawGeometryAndLights(vk::CommandBuffer cmdBuffer, SceneRenderDesc& sceneDesc);

} * Renderer{};
} // namespace vl
