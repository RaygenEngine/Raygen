#pragma once

#include "ecs_universe/BasicComponent.h"
struct Scene_;

namespace ed {
struct EditorCamera {

	REFLECTED_GENERIC(EditorCamera)
	{
		REFLECT_ICON(FA_CAMERA_RETRO);

		REFLECT_VAR(near);
		REFLECT_VAR(far);
		REFLECT_VAR(focalLength);
		REFLECT_VAR(vFov, PropertyFlags::Rads);
		REFLECT_VAR(hFov, PropertyFlags::Rads);
		REFLECT_VAR(vFovOffset, PropertyFlags::Rads);
		REFLECT_VAR(hFovOffset, PropertyFlags::Rads);
		REFLECT_VAR(viewportWidth, PropertyFlags::Transient);
		REFLECT_VAR(viewportHeight, PropertyFlags::Transient);
	}


	TransformCache transform;


	float focalLength{ 1.f };

	float vFov{ glm::radians(72.f) };
	float hFov{ glm::radians(106.f) };

	float near{ 0.1f };
	float far{ 1000.f };

	float vFovOffset{ 0.f };
	float hFovOffset{ 0.f };

	int32 viewportWidth{ 1280 };
	int32 viewportHeight{ 720 };


	//
	float orbitalLength{ 20.f };
	glm::vec3 orbitalCenter{};
	bool useOrbitalMode{ false };

	bool worldAlign{ false };

	float movementSpeed{ 5.f };
	float sensitivity{ 0.007f };

	glm::mat4 view;
	glm::mat4 proj;

	//
	void Update(float deltaSeconds);

	//
	void ResizeViewport(glm::uvec2 newSize);
	void ResetRotation();


	size_t sceneUid{ 0 };
	void InjectToScene(Scene_* worldScene);
	void EnqueueUpdateCmds(Scene_* worldScene);

private:
	void UpdateOrbital(float speed, float deltaSeconds);
	void UpdateFly(float speed, float deltaSeconds);
};
} // namespace ed
