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
		SetNodeScaleWCS(glm::vec3(1.f));
	}
	else {
		// if any change came here its from dragging in the editor.
		applyTo->SetNodeTransformWCS(GetNodeTransformWCS());
		SetNodeTransformLCS(glm::identity<glm::mat4>());
	}
	auto root = Engine::GetWorld()->GetRoot();


	if (input.IsKeyPressed(Key::X)) {
		m_worldAlign = !m_worldAlign;
	}

	if (input.IsKeyPressed(Key::C)) {
		auto pyr = applyTo->GetNodeEulerAnglesWCS();
		applyTo->SetNodeOrientationWCS(glm::identity<glm::quat>());
		applyTo->RotateNodeAroundAxisWCS(root->GetNodeUpWCS(), pyr.y);
	}

	// user rotation
	if (input.IsCursorDragged() && input.IsKeyRepeat(Key::RBUTTON)) {
		const float yaw = -input.GetCursorRelativePosition().x * m_turningSpeed * 0.5f;
		const float pitch = -input.GetCursorRelativePosition().y * m_turningSpeed * 0.5f;

		applyTo->RotateNodeAroundAxisWCS(root->GetNodeUpWCS(), yaw);
		applyTo->RotateNodeAroundAxisWCS(GetNodeRightWCS(), pitch);
	}

	if (!input.IsRightThumbResting()) {
		const auto yaw
			= -input.GetRightThumbDirection().x * input.GetRightThumbMagnitude() * 2.5f * m_turningSpeed * deltaTime;
		// upside down with regards to the cursor dragging
		const auto pitch
			= input.GetRightThumbDirection().y * input.GetRightThumbMagnitude() * 2.5f * m_turningSpeed * deltaTime;

		applyTo->RotateNodeAroundAxisWCS(root->GetNodeUpWCS(), glm::radians(yaw));
		applyTo->RotateNodeAroundAxisWCS(GetNodeRightWCS(), glm::radians(pitch));
	}

	// user movement
	if (!input.IsLeftThumbResting()) {
		// Calculate angle
		const float joystickAngle = atan2(-input.GetLeftThumbDirection().y * input.GetLeftThumbMagnitude(),
			input.GetLeftThumbDirection().x * input.GetLeftThumbMagnitude());

		// adjust angle to match user rotation
		const auto rotMat = glm::rotate(-(joystickAngle + glm::half_pi<float>()), GetNodeUpWCS());
		const glm::vec3 moveDir = rotMat * glm::vec4(GetNodeForwardWCS(), 1.f);

		applyTo->AddNodePositionOffsetLCS(moveDir * speed * input.GetLeftThumbMagnitude());
	}


	auto forward = GetNodeForwardWCS();
	auto right = GetNodeRightWCS();
	auto up = GetNodeUpWCS();

	if (m_worldAlign) {
		forward.y = 0.f;
		forward = glm::normalize(forward);

		right.y = 0.f;
		right = glm::normalize(right);

		up = root->GetNodeUpWCS();
	}

	if (input.IsAnyOfKeysRepeat(Key::W, Key::GAMEPAD_DPAD_UP)) {
		applyTo->AddNodePositionOffsetLCS(forward * speed);
	}

	if (input.IsAnyOfKeysRepeat(Key::S, Key::GAMEPAD_DPAD_DOWN)) {
		applyTo->AddNodePositionOffsetLCS((-forward) * speed);
	}

	if (input.IsAnyOfKeysRepeat(Key::D, Key::GAMEPAD_DPAD_RIGHT)) {
		applyTo->AddNodePositionOffsetLCS(right * speed);
	}

	if (input.IsAnyOfKeysRepeat(Key::A, Key::GAMEPAD_DPAD_LEFT)) {
		applyTo->AddNodePositionOffsetLCS((-right) * speed);
	}
	if (input.IsAnyOfKeysRepeat(Key::E, Key::GAMEPAD_RIGHT_SHOULDER)) {
		applyTo->AddNodePositionOffsetLCS(up * speed);
	}

	if (input.IsAnyOfKeysRepeat(Key::Q, Key::GAMEPAD_LEFT_SHOULDER)) {
		applyTo->AddNodePositionOffsetLCS((-up) * speed);
	}
}

void EditorCameraNode::ResetRotation()
{
	auto pyr = GetNodeEulerAnglesWCS();
	SetNodeOrientationWCS(glm::identity<glm::quat>());
	RotateNodeAroundAxisWCS(Engine::GetWorld()->GetRoot()->GetNodeUpWCS(), pyr.y);
}
