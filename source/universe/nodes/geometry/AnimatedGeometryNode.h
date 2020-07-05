#pragma once
#include "assets/pods/SkinnedMesh.h"
#include "assets/pods/Animation.h"
#include "universe/nodes/Node.h"

class AnimatedGeometryNode : public Node {
	REFLECTED_NODE(AnimatedGeometryNode, Node, DF_FLAGS(ModelChange, Joints))
	{
		REFLECT_ICON(FA_CUBE);
		REFLECT_VAR(m_skinnedMesh).OnDirty(DF::ModelChange);
		REFLECT_VAR(m_animation).OnDirty(DF::ModelChange);
		REFLECT_VAR(m_pushJoints).OnDirty(DF::Joints);
	}

	PodHandle<SkinnedMesh> m_skinnedMesh;
	PodHandle<Animation> m_animation;

	bool m_pushJoints;

	std::vector<glm::mat4> m_joints;

public:
	AnimatedGeometryNode();
	~AnimatedGeometryNode() override;


	void DirtyUpdate(DirtyFlagset dirtyFlags) override;

	void UpdateAnimation();

private:
	size_t sceneUid;
	template<typename Lambda>
	void Enqueue(Lambda&& l)
	{
		Scene->EnqueueCmd<typename SceneAnimatedGeometry>(sceneUid, l);
	}
};
