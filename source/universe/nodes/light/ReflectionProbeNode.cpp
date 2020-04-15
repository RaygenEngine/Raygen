#include "pch.h"
#include "ReflectionProbeNode.h"


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
			Enqueue([&](SceneReflectionProbe& rp) { rp.UploadCubemap(m_skybox); });
		}
	}

	if (flags[DF::AmbientTerm]) {
		Enqueue([&](SceneReflectionProbe& rp) { rp.ubo.color = glm::vec4(m_ambientTerm, 1.f); });
	}
}
