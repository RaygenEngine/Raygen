#include "EditorCamera.h"

#include "editor/Editor.h"
#include "engine/Engine.h"
#include "engine/Events.h"
#include "engine/Input.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneStructs.h"

namespace ed {

EditorCamera::EditorCamera()
{
	// XMStoreFloat3(&orbitalCenter, transform.translation() + orbitalLength * transform.front());

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
			// XMStoreFloat3(&orbitalCenter, transform.translation() + orbitalLength * transform.front());
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
	// float oldYaw = transform.yaw(); NEW::
	// transform.orientation = glm::angleAxis(glm::radians(oldYaw), engineSpaceUp);
	// transform.Compose();

	dirtyThisFrame = true;
}

void EditorCamera::InjectToScene(Scene* worldScene)
{
	// CHECK: Questionable usage of Enqueue cmd. If scene changes later to allow only one createdestroy per type this
	// will be an error
	worldScene->EnqueueCreateDestoryCmds<SceneCamera>({}, { &sceneUid });
}

void EditorCamera::EnqueueUpdateCmds(Scene* worldScene)
{
	if (!dirtyThisFrame) {
		return;
	}

	const XMVECTOR lookAt = XMVectorAdd(transform.translation(), XMVectorScale(transform.front(), orbitalLength));
	XMMATRIX view = XMMatrixLookAtRH(transform.translation(), lookAt, transform.up());

	const auto ar = static_cast<float>(viewportWidth) / static_cast<float>(viewportHeight);
	hFov = 2 * atan(ar * tan(vFov * 0.5f));

	const auto top = tan(vFov / 2.f + vFovOffset) * near;
	const auto bottom = tan(-vFov / 2.f - vFovOffset) * near;

	const auto right = tan(hFov / 2.f + hFovOffset) * near;
	const auto left = tan(-hFov / 2.f - hFovOffset) * near;


	XMMATRIX proj = XMMatrixPerspectiveOffCenterRH(left, right, bottom, top, near, far);

	const XMMATRIX _projInv = XMMatrixInverse(nullptr, proj);
	const XMMATRIX _viewInv = XMMatrixInverse(nullptr, view);

	// CHECK:
	// float filmWorldHeight = 2.0f * focalLength * tan(0.5f * vFov);
	// float filmWorldWidth = 2.0f * focalLength * tan(0.5f * hFov);
	// const auto filmArea = filmWorldHeight * filmWorldWidth;

	worldScene->EnqueueCmd<SceneCamera>(sceneUid, [=, translation = transform.translation()](SceneCamera& cam) {
		cam.prevViewProj = cam.ubo.viewProj;
		XMStoreFloat3A(&cam.ubo.position, translation);
		XMStoreFloat4x4A(&cam.ubo.view, view);
		XMStoreFloat4x4A(&cam.ubo.proj, proj);
		XMStoreFloat4x4A(&cam.ubo.viewProj, XMMatrixMultiply(view, proj));

		XMStoreFloat4x4A(&cam.ubo.viewInv, _viewInv);
		XMStoreFloat4x4A(&cam.ubo.projInv, _projInv);
		XMStoreFloat4x4A(&cam.ubo.viewProjInv, XMMatrixMultiply(_projInv, _viewInv));
	});

	dirtyThisFrame = false;
}

void EditorCamera::Focus(Entity entity)
{
	if (!entity) {
		return;
	}

	// auto pos = entity->world().position; NEW::

	// if (useOrbitalMode) {
	//	orbitalCenter = pos;
	//	OrbitalCenterChanged();
	//	return;
	//}

	// transform.position = pos - (transform.front() * orbitalLength);
	// transform.Compose();
	dirtyThisFrame = true;
}

void EditorCamera::TeleportToCamera(Entity entity)
{
	if (!entity) {
		return;
	}

	// entity->SetNodeTransformWCS( NEW::
	//	math::transformMat(entity.Basic().world().scale, transform.orientation, transform.position));
}

void EditorCamera::Pilot(Entity entity)
{
	if (entity == pilotEntity || !entity) {
		pilotEntity = {};
		return;
	}

	// transform = entity->world(); // NEW::
	// if (useOrbitalMode) {
	//	orbitalCenter = transform.position + transform.front() * orbitalLength;
	//	OrbitalCenterChanged();
	//}

	// dirtyThisFrame = true;

	// pilotEntity = entity;
}

void EditorCamera::MovePosition(FXMVECTOR offsetPos)
{
	XMVectorAdd(transform.translation(), offsetPos);
	transform.compose();

	if (useOrbitalMode) {
		// XMStoreFloat3(&orbitalCenter, transform.translation() + orbitalLength * transform.front());
		OrbitalCenterChanged();
	}
	dirtyThisFrame = true;
}

void EditorCamera::OrbitalCenterChanged()
{
	transform.set_translation(
		XMVectorSubtract(transform.translation(), XMVectorScale(transform.front(), orbitalLength)));
	transform.compose();
	dirtyThisFrame = true;
}

void EditorCamera::UpdatePiloting()
{
	// BUG: filter scale
	pilotEntity->SetNodeTransformWCS(transform.transform());
}

void EditorCamera::UpdateOrbital(float speed, float deltaSeconds)
{
	// auto& input = Input;

	// if (input.GetScrollDelta() != 0) {
	//	orbitalLength = std::max(orbitalLength - static_cast<float>(input.GetScrollDelta()), 1.0f);
	//	OrbitalCenterChanged();
	//}

	// if (input.IsMouseDragging()) {
	//	const auto yawPitch = glm::radians(-input.GetMouseDelta() * sensitivity);

	//	// yaw and pitch quats
	//	XMVECTOR yawRot = XMQuaternionRotationAxis(g_XMIdentityR1, yawPitch.x);
	//	XMVECTOR pitchRot = XMQuaternionRotationAxis(transform.right(), yawPitch.y);

	//	// newOrient = normalize(yawRot * (pitchRot * oldOrient))
	//	transform.set_orientation(XMQuaternionNormalize(
	//		XMQuaternionMultiply(yawRot, XMQuaternionMultiply(pitchRot, transform.orientation()))));


	//	if (!input.IsDown(Key::Mouse_LeftClick)) {
	//		// XMStoreFloat3(&orbitalCenter, transform.translation() + orbitalLength * transform.front());
	//		OrbitalCenterChanged();
	//	}
	//	else {
	//		transform.compose();
	//		dirtyThisFrame = true;
	//	}
	//}

	// if (input.IsDown(Key::Mouse_RightClick) || input.IsDown(Key::Mouse_LeftClick)) {
	//	// glm::vec3 front = transform.front(); NEW::
	//	// glm::vec3 right = transform.right();
	//	// glm::vec3 up = transform.up();

	//	// if (worldAlign) {
	//	//	front.y = 0.f;
	//	//	front = glm::normalize(front);

	//	//	right.y = 0.f;
	//	//	right = glm::normalize(right);

	//	//	up = engineSpaceUp;
	//	//}

	//	//if (input.AreKeysDown(Key::W /*, Key::GAMEPAD_DPAD_UP*/)) {
	//	//	XMStoreFloat3(&orbitalCenter, transform.translation() + orbitalLength * transform.front());
	//	//	pData->orbitalCenter = XMVectorAdd(transform.translation(), XMVectorScale(transform.front(), speed));
	//	//	dirtyThisFrame = true;
	//	//}

	//	//if (input.AreKeysDown(Key::S /*, Key::GAMEPAD_DPAD_DOWN*/)) {
	//	//	pData->orbitalCenter = XMVectorSubtract(transform.translation(), XMVectorScale(transform.front(), speed));
	//	//	dirtyThisFrame = true;
	//	//}

	//	//if (input.AreKeysDown(Key::D /*, Key::GAMEPAD_DPAD_RIGHT*/)) {
	//	//	pData->orbitalCenter = XMVectorAdd(transform.translation(), XMVectorScale(transform.right(), speed));
	//	//	dirtyThisFrame = true;
	//	//}

	//	//if (input.AreKeysDown(Key::A /*, Key::GAMEPAD_DPAD_LEFT*/)) {
	//	//	pData->orbitalCenter = XMVectorAdd(transform.translation(), XMVectorScale(transform.right(), speed));
	//	//	dirtyThisFrame = true;
	//	//}

	//	//if (input.AreKeysDown(Key::E /*, Key::GAMEPAD_RIGHT_SHOULDER*/)) {
	//	//	pData->orbitalCenter = XMVectorAdd(transform.translation(), XMVectorScale(transform.up(), speed));
	//	//	dirtyThisFrame = true;
	//	//}

	//	//if (input.AreKeysDown(Key::Q /*, Key::GAMEPAD_LEFT_SHOULDER*/)) {
	//	//	pData->orbitalCenter = XMVectorSubtract(transform.translation(), XMVectorScale(transform.up(), speed));
	//	//	dirtyThisFrame = true;
	//	//}


	//	if (dirtyThisFrame) {
	//		OrbitalCenterChanged();
	//	}
	//}
}


void EditorCamera::UpdateFly(float speed, float deltaSeconds)
{
	auto& input = Input;

	if (input.IsMouseDragging()) {
		auto yawPitch = glm::radians(-input.GetMouseDelta() * sensitivity);

		// yaw and pitch quats
		XMVECTOR yawRot = XMQuaternionRotationAxis(g_XMIdentityR1, yawPitch.x);
		XMVECTOR pitchRot = XMQuaternionRotationAxis(transform.right(), yawPitch.y);

		// newOrient = normalize(yawRot * (pitchRot * oldOrient))
		transform.set_orientation(XMQuaternionNormalize(
			XMQuaternionMultiply(XMQuaternionMultiply(transform.orientation(), pitchRot), yawRot)));
		transform.compose();
		dirtyThisFrame = true;
	}


	if (input.IsDown(Key::Mouse_RightClick)) {
		/*glm::vec3 front = transform.front(); NEW::
		glm::vec3 right = transform.right();
		glm::vec3 up = transform.up();

		if (worldAlign) {
			front.y = 0.f;
			front = glm::normalize(front);

			right.y = 0.f;
			right = glm::normalize(right);

			up = engineSpaceUp;
		}*/

		if (input.AreKeysDown(Key::W /*, Key::GAMEPAD_DPAD_UP*/)) {
			transform.set_translation(XMVectorAdd(transform.translation(), XMVectorScale(transform.front(), speed)));
			dirtyThisFrame = true;
		}

		if (input.AreKeysDown(Key::S /*, Key::GAMEPAD_DPAD_DOWN*/)) {
			transform.set_translation(
				XMVectorSubtract(transform.translation(), XMVectorScale(transform.front(), speed)));
			dirtyThisFrame = true;
		}

		if (input.AreKeysDown(Key::D /*, Key::GAMEPAD_DPAD_RIGHT*/)) {
			transform.set_translation(XMVectorAdd(transform.translation(), XMVectorScale(transform.right(), speed)));
			dirtyThisFrame = true;
		}

		if (input.AreKeysDown(Key::A /*, Key::GAMEPAD_DPAD_LEFT*/)) {
			transform.set_translation(
				XMVectorSubtract(transform.translation(), XMVectorScale(transform.right(), speed)));
			dirtyThisFrame = true;
		}

		if (input.AreKeysDown(Key::E /*, Key::GAMEPAD_RIGHT_SHOULDER*/)) {
			transform.set_translation(XMVectorAdd(transform.translation(), XMVectorScale(transform.up(), speed)));
			dirtyThisFrame = true;
		}

		if (input.AreKeysDown(Key::Q /*, Key::GAMEPAD_LEFT_SHOULDER*/)) {
			transform.set_translation(XMVectorSubtract(transform.translation(), XMVectorScale(transform.up(), speed)));
			dirtyThisFrame = true;
		}

		if (dirtyThisFrame) {
			transform.compose();
		}
	}
}
} // namespace ed
