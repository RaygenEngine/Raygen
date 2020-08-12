#include "pch.h"
#include "EditorCamera.h"


#include "engine/Input.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/Scene.h"

namespace ed {
void EditorCamera::Update(float deltaSeconds)
{

	auto& input = Input;
	auto& gamepad = input.GetGamepadState();

	// movementSpeed = glm::clamp(movementSpeed / 240.f * 2.0f, 2.0f, 100.0f);
	auto speed = movementSpeed;

	speed *= deltaSeconds;

	if (!gamepad.IsRTResting() || input.IsDown(Key::Shift)) {
		speed *= 5.f * glm::exp(gamepad.rt);
	}

	if (!gamepad.IsLTResting() || input.IsDown(Key::Ctrl)) {
		speed /= 5.f * glm::exp(gamepad.lt);
	}

	if (Input.IsJustPressed(Key::Z)) {
		useOrbitalMode = !useOrbitalMode;
		if (useOrbitalMode && orbitalLength <= 0) {
			orbitalLength = 20.f;
		}
		if (useOrbitalMode) {
			orbitalCenter = transform.forward() * orbitalLength;
		}
	}

	if (Input.IsJustPressed(Key::X)) {
		worldAlign = !worldAlign;
	}

	if (Input.IsJustPressed(Key::C)) {
		ResetRotation();
	}


	if (useOrbitalMode && orbitalLength > 0) {
		UpdateOrbital(speed, deltaSeconds);
	}
	else {
		UpdateFly(speed, deltaSeconds);
	}
}

void EditorCamera::ResizeViewport(glm::uvec2 newSize)
{
	newSize.x = std::max(newSize.x, 1u);
	newSize.y = std::max(newSize.y, 1u);

	viewportWidth = newSize.x;
	viewportHeight = newSize.y;
}

void EditorCamera::ResetRotation()
{
	float oldYaw = transform.pyr().y;
	transform.orientation = glm::angleAxis(glm::radians(oldYaw), engineSpaceUp);
	transform.Compose();
}

void EditorCamera::InjectToScene(Scene* worldScene)
{
	sceneUid = worldScene->EnqueueCreateCmd<SceneCamera>();
}

void EditorCamera::EnqueueUpdateCmds(Scene* worldScene)
{
	auto lookAt = transform.position + transform.forward() * focalLength;
	view = glm::lookAt(transform.position, lookAt, transform.up());

	auto viewInv = glm::inverse(view);

	glm::mat4 projInv;
	glm::mat4 viewProj;
	glm::mat4 viewProjInv;

	const auto ar = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
	hFov = 2 * atan(ar * tan(vFov * 0.5f));

	const auto top = tan(vFov / 2.f + vFovOffset) * near;
	const auto bottom = tan(-vFov / 2.f - vFovOffset) * near;

	const auto right = tan(hFov / 2.f + hFovOffset) * near;
	const auto left = tan(-hFov / 2.f - hFovOffset) * near;

	proj = glm::frustum(left, right, bottom, top, near, far);

	// Vulkan's inverted y
	proj[1][1] *= -1.f;

	projInv = glm::inverse(proj);
	viewProj = proj * view;
	viewProjInv = glm::inverse(viewProj);

	worldScene->EnqueueCmd<SceneCamera>(sceneUid, [=, pos = transform.position](SceneCamera& cam) {
		cam.ubo.position = glm::vec4(pos, 1.f);
		cam.ubo.view = view;
		cam.ubo.viewInv = viewInv;
		cam.ubo.proj = proj;
		cam.ubo.projInv = projInv;
		cam.ubo.viewProj = viewProj;
		cam.ubo.viewProjInv = viewProjInv;
	});

	proj[1][1] *= -1.f;
}

void EditorCamera::UpdateOrbital(float speed, float deltaSeconds)
{
	auto& input = Input;

	orbitalLength -= static_cast<float>(input.GetScrollDelta());


	if (input.IsDown(Key::Mouse_RightClick)) {
		glm::vec3 forward = transform.forward();
		glm::vec3 right = transform.right();
		glm::vec3 up = transform.up();


		if (worldAlign) {
			forward.y = 0.f;
			forward = glm::normalize(forward);

			right.y = 0.f;
			right = glm::normalize(right);

			up = engineSpaceUp;
		}


		if (input.AreKeysDown(Key::W /*, Key::GAMEPAD_DPAD_UP*/)) {
			orbitalCenter += forward * speed;
		}

		if (input.AreKeysDown(Key::S /*, Key::GAMEPAD_DPAD_DOWN*/)) {
			orbitalCenter -= forward * speed;
		}

		if (input.AreKeysDown(Key::D /*, Key::GAMEPAD_DPAD_RIGHT*/)) {
			orbitalCenter += right * speed;
		}

		if (input.AreKeysDown(Key::A /*, Key::GAMEPAD_DPAD_LEFT*/)) {
			orbitalCenter -= right * speed;
		}

		if (input.AreKeysDown(Key::E /*, Key::GAMEPAD_RIGHT_SHOULDER*/)) {
			orbitalCenter += up * speed;
		}

		if (input.AreKeysDown(Key::Q /*, Key::GAMEPAD_LEFT_SHOULDER*/)) {
			orbitalCenter -= up * speed;
		}
	}

	glm::quat orientation = transform.orientation;

	if (input.IsMouseDragging()) {
		const float yaw = -input.GetMouseDelta().x * sensitivity * 0.5f;
		const float pitch = -input.GetMouseDelta().y * sensitivity * 0.5f;

		auto pitchRot = glm::angleAxis(pitch, orientation * engineSpaceRight);
		auto yawRot = glm::angleAxis(yaw, engineSpaceUp);
		orientation = pitchRot * yawRot * orientation;
	}
	glm::mat4 centerMatrix = math::transformMat2(orbitalCenter, orientation);
	transform.transform = centerMatrix * math::transformMat2(-engineSpaceForward * orbitalLength);
	transform.Decompose();
}

void EditorCamera::UpdateFly(float speed, float deltaSeconds)
{

	auto& input = Input;

	// user rotation
	if (input.IsMouseDragging()) {
		const float yaw = -input.GetMouseDelta().x * sensitivity * 0.5f;
		const float pitch = -input.GetMouseDelta().y * sensitivity * 0.5f;

		auto pitchRot = glm::angleAxis(pitch, transform.right());
		auto yawRot = glm::angleAxis(yaw, engineSpaceUp);
		transform.orientation = pitchRot * yawRot * transform.orientation;
		transform.Compose();
	}


	if (input.IsDown(Key::Mouse_RightClick)) {
		glm::vec3 forward = transform.forward();
		glm::vec3 right = transform.right();
		glm::vec3 up = transform.up();

		if (worldAlign) {
			forward.y = 0.f;
			forward = glm::normalize(forward);

			right.y = 0.f;
			right = glm::normalize(right);

			up = engineSpaceUp;
		}

		if (input.AreKeysDown(Key::W /*, Key::GAMEPAD_DPAD_UP*/)) {
			transform.position += forward * speed;
		}

		if (input.AreKeysDown(Key::S /*, Key::GAMEPAD_DPAD_DOWN*/)) {
			transform.position -= forward * speed;
		}

		if (input.AreKeysDown(Key::D /*, Key::GAMEPAD_DPAD_RIGHT*/)) {
			transform.position += right * speed;
		}

		if (input.AreKeysDown(Key::A /*, Key::GAMEPAD_DPAD_LEFT*/)) {
			transform.position -= right * speed;
		}

		if (input.AreKeysDown(Key::E /*, Key::GAMEPAD_RIGHT_SHOULDER*/)) {
			transform.position += up * speed;
		}

		if (input.AreKeysDown(Key::Q /*, Key::GAMEPAD_LEFT_SHOULDER*/)) {
			transform.position -= up * speed;
		}
		transform.Compose();
	}
}
} // namespace ed
