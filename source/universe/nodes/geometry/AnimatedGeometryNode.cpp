#include "pch.h"
#include "AnimatedGeometryNode.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/scene/Scene.h"
#include "rendering/assets/GpuMesh.h"

AnimatedGeometryNode::AnimatedGeometryNode()
{
	sceneUid = Scene->EnqueueCreateCmd<SceneAnimatedGeometry>();
}

void AnimatedGeometryNode::DirtyUpdate(DirtyFlagset dirtyFlags)
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

AnimatedGeometryNode::~AnimatedGeometryNode()
{
	Scene->EnqueueDestroyCmd<SceneAnimatedGeometry>(sceneUid);
}
