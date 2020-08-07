#pragma once
#include "assets/PodHandle.h"
#include "universe/nodes/Node.h"

class AnimatedGeometryNode : public Node {
	REFLECTED_NODE(AnimatedGeometryNode, Node, DF_FLAGS(ModelChange, AnimationChange, Joints))
	{
		REFLECT_ICON(FA_CUBE);
		REFLECT_VAR(m_skinnedMesh).OnDirty(DF::ModelChange);
		REFLECT_VAR(m_animation).OnDirty(DF::AnimationChange);

		REFLECT_VAR(m_playbackSpeed);
		REFLECT_VAR(m_animationTime, PropertyFlags::Transient).OnDirty(DF::Joints);
		REFLECT_VAR(m_playInEditor, PropertyFlags::Transient);
	}

	PodHandle<SkinnedMesh> m_skinnedMesh;
	PodHandle<Animation> m_animation;


	std::vector<glm::mat4> m_joints;
	float m_animationTime{ 0 };

	bool m_playInEditor{ true };
	float m_playbackSpeed{ 1.0f };

public:
	AnimatedGeometryNode();
	~AnimatedGeometryNode() override;

	void Update(float deltaSeconds) override;


	void DirtyUpdate(DirtyFlagset dirtyFlags) override;

	void UpdateAnimation(float deltaSeconds);

	std::vector<glm::mat4> TickSamplers(float deltaTime);

private:
	size_t sceneUid;
	template<typename Lambda>
	void Enqueue(Lambda&& l)
	{
		//		Scene->EnqueueCmd<typename SceneAnimatedGeometry>(sceneUid, l);
	}
};
