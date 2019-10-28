#include "pch/pch.h"

#include "world/nodes/camera/EditorCameraNode.h"
#include "system/Engine.h"
#include "world/nodes/RootNode.h"
#include "system/Input.h"
#include "system/Engine.h"
#include "world/World.h"


void EditorCameraNode::UpdateFromEditor(float deltaTime)
{
	CameraNode::Update(deltaTime);

	auto& input = *Engine::GetInput();

	m_movementSpeed = glm::clamp(m_movementSpeed + input.GetWheelDelta() / 240.f * 2.0f, 2.0f, 100.0f);
	auto speed = m_movementSpeed;

	speed *= deltaTime;

	if (!input.IsRightTriggerResting() || input.IsKeyRepeat(Key::SHIFT)) {
		speed *= 5.f * glm::exp(input.GetRightTriggerMagnitude());
	}

	if (!input.IsLeftTriggerResting() || input.IsKeyRepeat(Key::CTRL)) {
		speed /= 5.f * glm::exp(input.GetLeftTriggerMagnitude());
	}

	Node* applyTo = GetParent();
	if (applyTo->IsRoot()) {
		applyTo = this;
		SetScale(glm::vec3(1.f));
	}
	else {
		// if any change came here its from dragging in the editor.
		applyTo->SetMatrix(GetMatrix());
		SetLocalMatrix(glm::identity<glm::mat4>());
	}
	auto root = Engine::GetWorld()->GetRoot();


	if (input.IsKeyPressed(Key::X)) {
		m_worldAlign = !m_worldAlign;
	}

	if (input.IsKeyPressed(Key::C)) {
		auto pyr = applyTo->GetEulerAngles();
		applyTo->SetOrientation(glm::identity<glm::quat>());
		applyTo->RotateAroundAxis(root->GetUp(), pyr.y);
	}

	// user rotation
	if (input.IsCursorDragged() && input.IsKeyRepeat(Key::RBUTTON)) {
		const float yaw = -input.GetCursorRelativePosition().x * m_turningSpeed * 0.5f;
		const float pitch = -input.GetCursorRelativePosition().y * m_turningSpeed * 0.5f;

		applyTo->RotateAroundAxis(root->GetUp(), yaw);
		applyTo->RotateAroundAxis(GetRight(), pitch);
	}

	if (!input.IsRightThumbResting()) {
		const auto yaw
			= -input.GetRightThumbDirection().x * input.GetRightThumbMagnitude() * 2.5f * m_turningSpeed * deltaTime;
		// upside down with regards to the cursor dragging
		const auto pitch
			= input.GetRightThumbDirection().y * input.GetRightThumbMagnitude() * 2.5f * m_turningSpeed * deltaTime;

		applyTo->RotateAroundAxis(root->GetUp(), glm::radians(yaw));
		applyTo->RotateAroundAxis(GetRight(), glm::radians(pitch));
	}

	// user movement
	if (!input.IsLeftThumbResting()) {
		// Calculate angle
		const float joystickAngle = atan2(-input.GetLeftThumbDirection().y * input.GetLeftThumbMagnitude(),
			input.GetLeftThumbDirection().x * input.GetLeftThumbMagnitude());

		// adjust angle to match user rotation
		const auto rotMat = glm::rotate(-(joystickAngle + glm::half_pi<float>()), GetUp());
		const glm::vec3 moveDir = rotMat * glm::vec4(GetForward(), 1.f);

		applyTo->AddLocalOffset(moveDir * speed * input.GetLeftThumbMagnitude());
	}


	auto forward = GetForward();
	auto right = GetRight();
	auto up = GetUp();

	if (m_worldAlign) {
		forward.y = 0.f;
		forward = glm::normalize(forward);

		right.y = 0.f;
		right = glm::normalize(right);

		up = root->GetUp();
	}

	if (input.IsAnyOfKeysRepeat(Key::W, Key::GAMEPAD_DPAD_UP)) {
		applyTo->AddLocalOffset(forward * speed);
	}

	if (input.IsAnyOfKeysRepeat(Key::S, Key::GAMEPAD_DPAD_DOWN)) {
		applyTo->AddLocalOffset((-forward) * speed);
	}

	if (input.IsAnyOfKeysRepeat(Key::D, Key::GAMEPAD_DPAD_RIGHT)) {
		applyTo->AddLocalOffset(right * speed);
	}

	if (input.IsAnyOfKeysRepeat(Key::A, Key::GAMEPAD_DPAD_LEFT)) {
		applyTo->AddLocalOffset((-right) * speed);
	}
	if (input.IsAnyOfKeysRepeat(Key::E, Key::GAMEPAD_RIGHT_SHOULDER)) {
		applyTo->AddLocalOffset(up * speed);
	}

	if (input.IsAnyOfKeysRepeat(Key::Q, Key::GAMEPAD_LEFT_SHOULDER)) {
		applyTo->AddLocalOffset((-up) * speed);
	}
}

void EditorCameraNode::ResetRotation()
{
	auto pyr = GetEulerAngles();
	SetOrientation(glm::identity<glm::quat>());
	RotateAroundAxis(Engine::GetWorld()->GetRoot()->GetUp(), pyr.y);
}
