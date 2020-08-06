#pragma once
#include "ecs_universe/BasicComponent.h"
#include "ecs_universe/ComponentsDb.h"
#include "ecs_universe/SceneComponentBase.h"

struct SceneCamera;

struct CCamera : CSceneBase {
	REFLECTED_SCENE_COMP(CCamera, SceneCamera)
	{
		REFLECT_ICON(FA_CAMERA_RETRO);
		REFLECT_CATEGORY("Rendering");

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

	// distance to film plane
	float focalLength{ 1.f };

	// vertical fov (angle)
	float vFov{ glm::radians(72.f) };
	// horizontal fov depends on the vertical and the aspect ratio
	float hFov{ glm::radians(106.f) };

	float near{ 0.1f };
	float far{ 1000.f };

	float vFovOffset{ 0.f };
	float hFovOffset{ 0.f };

	int32 viewportWidth{ 1280 };
	int32 viewportHeight{ 720 };
};
