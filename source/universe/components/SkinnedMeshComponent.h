#pragma once

#include "universe/ComponentsDb.h"
#include "universe/SceneComponentBase.h"


struct SceneAnimatedGeometry;

// TODO:
struct CSkinnedMesh : CSceneBase {

	REFLECTED_SCENE_COMP(CSkinnedMesh, SceneAnimatedGeometry)
	{
		REFLECT_ICON(FA_RUNNING);
		REFLECT_VAR(skinnedMesh);
		REFLECT_VAR(animation);

		REFLECT_VAR(playbackSpeed);
		REFLECT_VAR(animationTime, PropertyFlags::Transient);
		REFLECT_VAR(playInEditor, PropertyFlags::Transient);
	}

	PodHandle<SkinnedMesh> skinnedMesh;
	PodHandle<Animation> animation;

	float playbackSpeed{ 1.0f };

	// Current Running animation 'frame' in seconds, NOT total animation time.
	float animationTime{ 0 };
	bool playInEditor{ true };

	std::vector<glm::mat4> joints;
};
