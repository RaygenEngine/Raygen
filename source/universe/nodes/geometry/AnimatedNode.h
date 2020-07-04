#pragma once
#include "assets/pods/Mesh.h"
#include "universe/nodes/Node.h"

class AnimatedNode : public Node {
	REFLECTED_NODE(AnimatedNode, Node, DF_FLAGS(ModelChange))
	{
		REFLECT_ICON(FA_CUBE);
		REFLECT_VAR(m_skinnedMesh).OnDirty(DF::ModelChange);
		REFLECT_VAR(m_animation).OnDirty(DF::ModelChange);
	}

public:
	PodHandle<SkinnedMesh> m_skinnedMesh;
	PodHandle<Animation> m_animation;


	AnimatedNode();
	~AnimatedNode() override;


	void DirtyUpdate(DirtyFlagset dirtyFlags) override;

private:
	size_t sceneUid;
	template<typename Lambda>
	void Enqueue(Lambda&& l)
	{
		Scene->EnqueueCmd<typename SceneAnimatedGeometry>(sceneUid, l);
	}
};
