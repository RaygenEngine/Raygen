#include "pch.h"
#include "ReflectionProbeNode.h"

#include "rendering/assets/GpuAssetManager.h"


ReflectionProbeNode::ReflectionProbeNode()
{
	sceneUid = Scene->EnqueueCreateCmd<SceneReflectionProbe>();
}

ReflectionProbeNode::~ReflectionProbeNode()
{
	Scene->EnqueueDestroyCmd<SceneReflectionProbe>(sceneUid);
}


void ReflectionProbeNode::DirtyUpdate(DirtyFlagset flags)
{
	Node::DirtyUpdate(flags);
	if (flags[DF::SkyTexture]) {

		if (m_skybox.Lock()->width > 0) {
			Enqueue([cubemap = m_skybox](
						SceneReflectionProbe& rp) { rp.cubemap = vl::GpuAssetManager->GetGpuHandle(cubemap); });
		}
	}

	if (flags[DF::AmbientTerm]) {
		Enqueue(
			[ambientTerm = m_ambientTerm](SceneReflectionProbe& rp) { rp.ubo.color = glm::vec4(ambientTerm, 1.f); });
	}
}
