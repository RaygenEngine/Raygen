#include "pch/pch.h"

#include "world/nodes/camera/EditorCameraNode.h"
#include "system/Engine.h"
#include "world/nodes/RootNode.h"
#include "system/Input.h"
#include "system/Engine.h"
#include "world/World.h"


EditorCameraNode::EditorCameraNode()
	: WindowCameraNode()
{
	RecalculateProjectionFov();
}

void EditorCameraNode::UpdateFromEditor(float deltaTime)
{
	CameraNode::Update(deltaTime);

	auto& input = *Engine::GetInput();

	m_movementSpeed = glm::clamp(m_movementSpeed + input.GetWheelDelta() / 240.f * 2.0f, 2.0f, 100.0f);
	auto speed = m_movementSpeed;

	speed *= deltaTime;

	if (!input.IsRightTriggerResting() || input.IsKeyRepeat(XVirtualKey::SHIFT)) {
		speed *= 5.f * glm::exp(input.GetRightTriggerMagnitude());
	}

	if (!input.IsLeftTriggerResting() || input.IsKeyRepeat(XVirtualKey::CTRL)) {
		speed /= 5.f * glm::exp(input.GetLeftTriggerMagnitude());
	}

	Node* applyTo = GetParent();
	if (applyTo->IsRoot()) {
		applyTo = this;
		SetWorldScale(glm::vec3(1.f));
	}
	else {
		// if any change came here its from dragging in the editor.
		applyTo->SetWorldMatrix(GetWorldMatrix());
		SetLocalMatrix(glm::identity<glm::mat4>());
	}
	auto root = Engine::GetWorld()->GetRoot();


	if (input.IsKeyPressed(XVirtualKey::X)) {
		m_worldAlign = !m_worldAlign;
	}

	if (input.IsKeyPressed(XVirtualKey::C)) {
		auto pyr = applyTo->GetWorldPYR();
		applyTo->SetWorldOrientation(glm::identity<glm::quat>());
		applyTo->RotateAroundAxis(root->GetWorldUp(), pyr.y);
	}

	// user rotation
	if (input.IsCursorDragged() && input.IsKeyRepeat(XVirtualKey::RBUTTON)) {
		const float yaw = -input.GetCursorRelativePosition().x * m_turningSpeed * 0.5f;
		const float pitch = -input.GetCursorRelativePosition().y * m_turningSpeed * 0.5f;

		applyTo->RotateAroundAxis(root->GetWorldUp(), yaw);
		applyTo->RotateAroundAxis(GetWorldRight(), pitch);
	}

	if (!input.IsRightThumbResting()) {
		const auto yaw
			= -input.GetRightThumbDirection().x * input.GetRightThumbMagnitude() * 2.5f * m_turningSpeed * deltaTime;
		// upside down with regards to the cursor dragging
		const auto pitch
			= input.GetRightThumbDirection().y * input.GetRightThumbMagnitude() * 2.5f * m_turningSpeed * deltaTime;

		applyTo->RotateAroundAxis(root->GetWorldUp(), glm::radians(yaw));
		applyTo->RotateAroundAxis(GetWorldRight(), glm::radians(pitch));
	}

	// user movement
	if (!input.IsLeftThumbResting()) {
		// Calculate angle
		const float joystickAngle = atan2(-input.GetLeftThumbDirection().y * input.GetLeftThumbMagnitude(),
			input.GetLeftThumbDirection().x * input.GetLeftThumbMagnitude());

		// adjust angle to match user rotation
		const auto rotMat = glm::rotate(-(joystickAngle + glm::half_pi<float>()), GetWorldUp());
		const glm::vec3 moveDir = rotMat * glm::vec4(GetWorldForward(), 1.f);

		applyTo->AddLocalOffset(moveDir * speed * input.GetLeftThumbMagnitude());
	}


	auto forward = GetWorldForward();
	auto right = GetWorldRight();
	auto up = GetWorldUp();

	if (m_worldAlign) {
		forward.y = 0.f;
		forward = glm::normalize(forward);

		right.y = 0.f;
		right = glm::normalize(right);

		up = root->GetWorldUp();
	}

	if (input.IsAnyOfKeysRepeat(XVirtualKey::W, XVirtualKey::GAMEPAD_DPAD_UP)) {
		applyTo->AddLocalOffset(forward * speed);
	}

	if (input.IsAnyOfKeysRepeat(XVirtualKey::S, XVirtualKey::GAMEPAD_DPAD_DOWN)) {
		applyTo->AddLocalOffset((-forward) * speed);
	}

	if (input.IsAnyOfKeysRepeat(XVirtualKey::D, XVirtualKey::GAMEPAD_DPAD_RIGHT)) {
		applyTo->AddLocalOffset(right * speed);
	}

	if (input.IsAnyOfKeysRepeat(XVirtualKey::A, XVirtualKey::GAMEPAD_DPAD_LEFT)) {
		applyTo->AddLocalOffset((-right) * speed);
	}
	if (input.IsAnyOfKeysRepeat(XVirtualKey::E, XVirtualKey::GAMEPAD_RIGHT_SHOULDER)) {
		applyTo->AddLocalOffset(up * speed);
	}

	if (input.IsAnyOfKeysRepeat(XVirtualKey::Q, XVirtualKey::GAMEPAD_LEFT_SHOULDER)) {
		applyTo->AddLocalOffset((-up) * speed);
	}
}

void EditorCameraNode::ResetRotation()
{
	auto pyr = GetWorldPYR();
	SetWorldOrientation(glm::identity<glm::quat>());
	RotateAroundAxis(Engine::GetWorld()->GetRoot()->GetWorldUp(), pyr.y);
}

void EditorCameraNode::WindowResize(int32 x, int32 y)
{
	WindowCameraNode::WindowResize(x, y);
	RecalculateProjectionFov();
}
