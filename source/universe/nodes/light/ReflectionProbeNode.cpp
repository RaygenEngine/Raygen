#include "pch.h"
#include "ReflectionProbeNode.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/scene/SceneReflectionProbe.h"

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
	if (flags[DF::EnvMap]) {

		Enqueue([envmap = m_environmentMap](
					SceneReflectionProbe& rp) { rp.envmap = vl::GpuAssetManager->GetGpuHandle(envmap); });
	}

	if (flags[DF::AmbientTerm]) {
		Enqueue(
			[ambientTerm = m_ambientTerm](SceneReflectionProbe& rp) { rp.ubo.color = glm::vec4(ambientTerm, 1.f); });
	}
}
