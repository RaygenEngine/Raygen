#include "pch.h"
#include "AnimatedGeometryNode.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/scene/Scene.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/assets/GpuSkinnedMesh.h"


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
			geom.modelPod = model;
			geom.model = vl::GpuAssetManager->GetGpuHandle(model);
			// geom.animation = vl::GpuAssetManager->GetGpuHandle(anim);
		});

		m_joints = m_skinnedMesh.Lock()->jointMatrices;
		m_joints.resize(2);
	}

	if (dirtyFlags[DF::SRT]) {
		Enqueue([trans = GetNodeTransformWCS()](SceneAnimatedGeometry& geom) { geom.transform = trans; });
	}

	if (dirtyFlags[DF::Joints] && !dirtyFlags[DF::ModelChange]) {
		Enqueue([joints = m_joints](SceneAnimatedGeometry& geom) {
			geom.ubo.jointMatrices[0] = joints[0];
			geom.ubo.jointMatrices[1] = joints[1];
		});
		UpdateAnimation();
	}
}

void AnimatedGeometryNode::UpdateAnimation()
{
	static int32 currentFrame = 0;

	auto& channels = m_animation.Lock()->channels;
	auto& samplers = m_animation.Lock()->samplers;

	for (auto& channel : channels) {
		AnimationPath path = channel.path;

		auto& sampler = samplers[channel.samplerIndex];
		const glm::quat* data = reinterpret_cast<const glm::quat*>(sampler.outputs.data());

		const auto r = glm::toMat4(data[currentFrame]);
		m_joints[1] = r * m_skinnedMesh.Lock()->jointMatrices[1];
		currentFrame = (currentFrame + 1) % (sampler.outputs.size() / sizeof(glm::quat));
	}
}

AnimatedGeometryNode::~AnimatedGeometryNode()
{
	Scene->EnqueueDestroyCmd<SceneAnimatedGeometry>(sceneUid);
}
