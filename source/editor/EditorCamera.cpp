#include "EditorCamera.h"

#include "editor/Editor.h"
#include "engine/Engine.h"
#include "engine/Events.h"
#include "engine/Input.h"
#include "rendering/Renderer.h" // WIP: remove
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneCamera.h"

namespace ed {

EditorCamera::EditorCamera()
{
	orbitalCenter = transform.position + transform.front() * orbitalLength;
	Event::OnViewportUpdated.Bind(this, [&]() { ResizeViewport(g_ViewportCoordinates.size); });
}


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
			orbitalLength = 5.f;
		}

		if (useOrbitalMode) {
			orbitalCenter = transform.position + transform.front() * orbitalLength;
		}
	}

	if (Input.IsJustPressed(Key::X)) {
		worldAlign = !worldAlign;
	}

	if (Input.IsJustPressed(Key::C)) {
		ResetRotation();
	}


	if (Input.IsJustPressed(Key::F)) {
		if (Input.IsDown(Key::Shift)) {
			Pilot(Editor::GetSelection());
		}
		else {
			Focus(Editor::GetSelection());
		}
	}


	if (useOrbitalMode && orbitalLength > 0) {
		UpdateOrbital(speed, deltaSeconds);
	}
	else {
		UpdateFly(speed, deltaSeconds);
	}

	if (pilotEntity && dirtyThisFrame) {
		UpdatePiloting();
	}

	// WIP : remove
	if (dirtyThisFrame) {
		vl::Renderer->m_raytraceArealights.frame = 0;
		vl::Renderer->m_progressivePathtrace.frame = 0;
	}

	dirtyThisFrame = true;
}


void EditorCamera::ResizeViewport(glm::uvec2 newSize)
{
	newSize.x = std::max(newSize.x, 1u);
	newSize.y = std::max(newSize.y, 1u);

	viewportWidth = newSize.x;
	viewportHeight = newSize.y;

	dirtyThisFrame = true;
}

void EditorCamera::ResetRotation()
{
	float oldYaw = transform.yaw();
	transform.orientation = glm::angleAxis(glm::radians(oldYaw), engineSpaceUp);
	transform.Compose();

	dirtyThisFrame = true;
}

void EditorCamera::InjectToScene(Scene* worldScene)
{
	// CHECK: Questionable usage of Enqueu cmd. If scene changes later to allow only one createdestroy per type this
	// will be an error
	worldScene->EnqueueCreateDestoryCmds<SceneCamera>({}, { &sceneUid });
}

void EditorCamera::EnqueueUpdateCmds(Scene* worldScene)
{
	if (!dirtyThisFrame) {
		return;
	}
	auto lookAt = transform.position + transform.front() * focalLength;
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
	viewProjInv = viewInv * projInv;

	worldScene->EnqueueCmd<SceneCamera>(sceneUid, [=, pos = transform.position](SceneCamera& cam) {
		cam.prevViewProj = cam.ubo.viewProj;
		cam.ubo.position = glm::vec4(pos, 1.f);
		cam.ubo.view = view;
		cam.ubo.viewInv = viewInv;
		cam.ubo.proj = proj;
		cam.ubo.projInv = projInv;
		cam.ubo.viewProj = viewProj;
		cam.ubo.viewProjInv = viewProjInv;
	});

	proj[1][1] *= -1.f;
	dirtyThisFrame = false;
}

void EditorCamera::Focus(Entity entity)
{
	if (!entity) {
		return;
	}

	auto pos = entity->world().position;

	if (useOrbitalMode) {
		orbitalCenter = pos;
		OrbitalCenterChanged();
		return;
	}

	transform.position = pos - (transform.front() * orbitalLength);
	transform.Compose();
	dirtyThisFrame = true;
}

void EditorCamera::TeleportToCamera(Entity entity)
{
	if (!entity) {
		return;
	}
	// BUG: filter scale
	entity->SetNodeTransformWCS(transform.transform);
}

void EditorCamera::Pilot(Entity entity)
{
	if (entity == pilotEntity || !entity) {
		pilotEntity = {};
		return;
	}

	transform = entity->world();
	if (useOrbitalMode) {
		orbitalCenter = transform.position + transform.front() * orbitalLength;
		OrbitalCenterChanged();
	}

	dirtyThisFrame = true;

	pilotEntity = entity;
}

void EditorCamera::MovePosition(glm::vec3 offsetPos)
{
	transform.position += offsetPos;
	transform.Compose();

	if (useOrbitalMode) {
		orbitalCenter = transform.position + transform.front() * orbitalLength;
		OrbitalCenterChanged();
	}
	dirtyThisFrame = true;
}

void EditorCamera::OrbitalCenterChanged()
{
	transform.position = orbitalCenter - (transform.front() * orbitalLength);
	transform.Compose();
	dirtyThisFrame = true;
}

void EditorCamera::UpdatePiloting()
{
	// BUG: filter scale
	pilotEntity->SetNodeTransformWCS(transform.transform);
}

void EditorCamera::UpdateOrbital(float speed, float deltaSeconds)
{
	auto& input = Input;

	if (input.GetScrollDelta() != 0) {
		orbitalLength = std::max(orbitalLength - static_cast<float>(input.GetScrollDelta()), 1.0f);
		OrbitalCenterChanged();
	}

	if (input.IsMouseDragging()) {
		const auto yawPitch = glm::radians(-input.GetMouseDelta() * sensitivity);

		auto yawRot = glm::angleAxis(yawPitch.x, engineSpaceUp);
		auto pitchRot = glm::angleAxis(yawPitch.y, transform.right());

		transform.orientation = glm::normalize(yawRot * (pitchRot * transform.orientation));

		if (!input.IsDown(Key::Mouse_LeftClick)) {
			orbitalCenter = transform.position + transform.front() * orbitalLength;
			OrbitalCenterChanged();
		}
		else {
			transform.Compose();
			dirtyThisFrame = true;
		}
	}

	if (input.IsDown(Key::Mouse_RightClick) || input.IsDown(Key::Mouse_LeftClick)) {
		glm::vec3 front = transform.front();
		glm::vec3 right = transform.right();
		glm::vec3 up = transform.up();

		if (worldAlign) {
			front.y = 0.f;
			front = glm::normalize(front);

			right.y = 0.f;
			right = glm::normalize(right);

			up = engineSpaceUp;
		}

		if (input.AreKeysDown(Key::W /*, Key::GAMEPAD_DPAD_UP*/)) {
			orbitalCenter += front * speed;
			dirtyThisFrame = true;
		}

		if (input.AreKeysDown(Key::S /*, Key::GAMEPAD_DPAD_DOWN*/)) {
			orbitalCenter -= front * speed;
			dirtyThisFrame = true;
		}

		if (input.AreKeysDown(Key::D /*, Key::GAMEPAD_DPAD_RIGHT*/)) {
			orbitalCenter += right * speed;
			dirtyThisFrame = true;
		}

		if (input.AreKeysDown(Key::A /*, Key::GAMEPAD_DPAD_LEFT*/)) {
			orbitalCenter -= right * speed;
			dirtyThisFrame = true;
		}

		if (input.AreKeysDown(Key::E /*, Key::GAMEPAD_RIGHT_SHOULDER*/)) {
			orbitalCenter += up * speed;
			dirtyThisFrame = true;
		}

		if (input.AreKeysDown(Key::Q /*, Key::GAMEPAD_LEFT_SHOULDER*/)) {
			orbitalCenter -= up * speed;
			dirtyThisFrame = true;
		}


		if (dirtyThisFrame) {
			OrbitalCenterChanged();
		}
	}
}


void EditorCamera::UpdateFly(float speed, float deltaSeconds)
{
	auto& input = Input;

	if (input.IsMouseDragging()) {
		auto yawPitch = glm::radians(-input.GetMouseDelta() * sensitivity);

		auto yawRot = glm::angleAxis(yawPitch.x, engineSpaceUp);
		auto pitchRot = glm::angleAxis(yawPitch.y, transform.right());

		transform.orientation = glm::normalize(yawRot * (pitchRot * transform.orientation));
		transform.Compose();
		dirtyThisFrame = true;
	}


	if (input.IsDown(Key::Mouse_RightClick)) {
		glm::vec3 front = transform.front();
		glm::vec3 right = transform.right();
		glm::vec3 up = transform.up();

		if (worldAlign) {
			front.y = 0.f;
			front = glm::normalize(front);

			right.y = 0.f;
			right = glm::normalize(right);

			up = engineSpaceUp;
		}

		const auto oldPos = transform.position;

		if (input.AreKeysDown(Key::W /*, Key::GAMEPAD_DPAD_UP*/)) {
			transform.position += front * speed;
			dirtyThisFrame = true;
		}

		if (input.AreKeysDown(Key::S /*, Key::GAMEPAD_DPAD_DOWN*/)) {
			transform.position -= front * speed;
			dirtyThisFrame = true;
		}

		if (input.AreKeysDown(Key::D /*, Key::GAMEPAD_DPAD_RIGHT*/)) {
			transform.position += right * speed;
			dirtyThisFrame = true;
		}

		if (input.AreKeysDown(Key::A /*, Key::GAMEPAD_DPAD_LEFT*/)) {
			transform.position -= right * speed;
			dirtyThisFrame = true;
		}

		if (input.AreKeysDown(Key::E /*, Key::GAMEPAD_RIGHT_SHOULDER*/)) {
			transform.position += up * speed;
			dirtyThisFrame = true;
		}

		if (input.AreKeysDown(Key::Q /*, Key::GAMEPAD_LEFT_SHOULDER*/)) {
			transform.position -= up * speed;
			dirtyThisFrame = true;
		}

		if (dirtyThisFrame) {
			transform.Compose();
		}
	}
}
} // namespace ed
