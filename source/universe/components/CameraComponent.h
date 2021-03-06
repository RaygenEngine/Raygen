#pragma once
#include "universe/BasicComponent.h"
#include "universe/ComponentsDb.h"
#include "universe/SceneComponentBase.h"

struct SceneCamera;

struct CCamera : CSceneBase {
	REFLECTED_SCENE_COMP(CCamera, SceneCamera)
	{
		REFLECT_ICON(FA_CAMERA);
		// REFLECT_CATEGORY("Rendering");

		REFLECT_VAR(near).Clamp(0.001f);
		REFLECT_VAR(far).Clamp(0.001f);
		REFLECT_VAR(focalLength).Clamp(0.001f);
		REFLECT_VAR(vFov, PropertyFlags::Rads).Clamp(0.001f, glm::pi<float>());
		REFLECT_VAR(hFov, PropertyFlags::Rads).Clamp(0.001f, glm::pi<float>());
		REFLECT_VAR(vFovOffset, PropertyFlags::Rads);
		REFLECT_VAR(hFovOffset, PropertyFlags::Rads);
		REFLECT_VAR(viewportWidth, PropertyFlags::Transient).Clamp(1.f);
		REFLECT_VAR(viewportHeight, PropertyFlags::Transient).Clamp(1.f);
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

	glm::mat4 view;
	glm::mat4 proj;
};
