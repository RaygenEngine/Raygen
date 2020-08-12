#pragma once

struct Scene_;

struct AnimatorSystem {

public:
	static void UpdateAnimations(entt::registry& reg, float deltaSeconds);

	static void UploadAnimationsToScene(entt::registry& reg, Scene_* scene);
};
