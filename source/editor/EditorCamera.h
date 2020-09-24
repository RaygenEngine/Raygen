#pragma once

#include "universe/BasicComponent.h"
#include "engine/Listener.h"
struct Scene;

namespace ed {
struct EditorCamera : public Listener {

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
	EditorCamera();

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
	float orbitalLength{ 5.f };
	glm::vec3 orbitalCenter{};
	bool useOrbitalMode{ true };

	bool worldAlign{ false };

	bool dirtyThisFrame{ false };

	float movementSpeed{ 5.f };

	// In Degrees per mouse pixel movement. (ie: sensitivity of 0.5 would mean 0.5 degrees rotation for 1 unit of mouse
	// movement)
	float sensitivity{ 0.15f };

	glm::mat4 view;
	glm::mat4 proj;

	//
	void Update(float deltaSeconds);

	//
	void ResizeViewport(glm::uvec2 newSize);
	void ResetRotation();


	size_t sceneUid{ 0 };
	void InjectToScene(Scene* worldScene);
	void EnqueueUpdateCmds(Scene* worldScene);

public:
	void Focus(Entity entity);
	void TeleportToCamera(Entity entity);
	void Pilot(Entity entity);


private:
	Entity pilotEntity{};

	void OrbitalCenterChanged();
	void UpdatePiloting();

	void UpdateOrbital(float speed, float deltaSeconds);
	void UpdateFly(float speed, float deltaSeconds);
};
} // namespace ed
