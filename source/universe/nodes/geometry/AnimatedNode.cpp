#include "pch.h"
#include "AnimatedNode.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/scene/Scene.h"
#include "rendering/assets/GpuMesh.h"
#include "assets/pods/SkinnedMesh.h"

AnimatedNode::AnimatedNode()
{
	sceneUid = Scene->EnqueueCreateCmd<SceneAnimatedGeometry>();
}

void AnimatedNode::DirtyUpdate(DirtyFlagset dirtyFlags)
{
	Node::DirtyUpdate(dirtyFlags);

	if (dirtyFlags[DF::ModelChange]) {
		CalculateWorldAABB();

		Enqueue([model = m_skinnedMesh, anim = m_animation](SceneAnimatedGeometry& geom) {
			geom.model = vl::GpuAssetManager->GetGpuHandle(model);
			geom.animation = vl::GpuAssetManager->GetGpuHandle(anim);
			geom.ubo.jointMatrices[0] = model.Lock()->jointMatrices[0];
			geom.ubo.jointMatrices[1] = model.Lock()->jointMatrices[1];
		});
	}

	if (dirtyFlags[DF::SRT]) {
		Enqueue([trans = GetNodeTransformWCS()](SceneAnimatedGeometry& geom) { geom.transform = trans; });
	}
}

AnimatedNode::~AnimatedNode()
{
	Scene->EnqueueDestroyCmd<SceneAnimatedGeometry>(sceneUid);
}
