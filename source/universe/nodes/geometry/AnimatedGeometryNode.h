#pragma once
#include "assets/pods/SkinnedMesh.h"
#include "assets/pods/Animation.h"
#include "universe/nodes/Node.h"

class AnimatedGeometryNode : public Node {
	REFLECTED_NODE(AnimatedGeometryNode, Node, DF_FLAGS(ModelChange))
	{
		REFLECT_ICON(FA_CUBE);
		REFLECT_VAR(m_skinnedMesh).OnDirty(DF::ModelChange);
		REFLECT_VAR(m_animation).OnDirty(DF::ModelChange);
	}

	PodHandle<SkinnedMesh> m_skinnedMesh;
	PodHandle<Animation> m_animation;

public:
	AnimatedGeometryNode();
	~AnimatedGeometryNode() override;


	void DirtyUpdate(DirtyFlagset dirtyFlags) override;

private:
	size_t sceneUid;
	template<typename Lambda>
	void Enqueue(Lambda&& l)
	{
		Scene->EnqueueCmd<typename SceneAnimatedGeometry>(sceneUid, l);
	}
};
