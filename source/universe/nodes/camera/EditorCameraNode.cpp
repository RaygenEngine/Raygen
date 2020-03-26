#include "pch.h"
#include "EditorCameraNode.h"

#include "engine/Engine.h"
#include "engine/Input.h"
#include "universe/nodes/RootNode.h"
#include "universe/Universe.h"


void EditorCameraNode::UpdateFromEditor(float deltaTime)
{
	CameraNode::Update(deltaTime);

	auto& input = Engine.GetInput();
	auto& gamepad = input.GetGamepadState();

	m_movementSpeed = glm::clamp(m_movementSpeed + input.GetScrollDelta() / 240.f * 2.0f, 2.0f, 100.0f);
	auto speed = m_movementSpeed;

	speed *= deltaTime;

	if (!gamepad.IsRTResting() || input.IsDown(Key::Shift)) {
		speed *= 5.f * glm::exp(gamepad.rt);
	}

	if (!gamepad.IsLTResting() || input.IsDown(Key::Ctrl)) {
		speed /= 5.f * glm::exp(gamepad.lt);
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
	auto root = Universe::MainWorld->GetRoot();

	if (input.IsJustPressed(Key::X)) {
		m_worldAlign = !m_worldAlign;
	}

	if (input.IsJustPressed(Key::C)) {
		auto pyr = applyTo->GetNodeEulerAnglesWCS();
		applyTo->SetNodeOrientationWCS(glm::identity<glm::quat>());
		applyTo->RotateNodeAroundAxisWCS(root->GetNodeUpWCS(), pyr.y);
	}

	// user rotation
	if (input.IsMouseDragging()) {
		const float yaw = -input.GetMouseDelta().x * m_turningSpeed * 0.5f;
		const float pitch = -input.GetMouseDelta().y * m_turningSpeed * 0.5f;

		applyTo->RotateNodeAroundAxisWCS(root->GetNodeUpWCS(), yaw);
		applyTo->RotateNodeAroundAxisWCS(GetNodeRightWCS(), pitch);
	}

	if (!gamepad.IsRTResting()) {
		const auto yaw = -gamepad.rs.GetXAxisValue() * 2.5f * m_turningSpeed * deltaTime;
		// upside down with regards to the cursor dragging
		const auto pitch = -gamepad.rs.GetYAxisValue() * 2.5f * m_turningSpeed * deltaTime;

		applyTo->RotateNodeAroundAxisWCS(root->GetNodeUpWCS(), glm::radians(yaw));
		applyTo->RotateNodeAroundAxisWCS(GetNodeRightWCS(), glm::radians(pitch));
	}

	// user movement
	if (!gamepad.IsLTResting()) {
		// Calculate angle
		const float joystickAngle = atan2(-gamepad.ls.GetYAxisValue(), gamepad.ls.GetXAxisValue());

		// adjust angle to match user rotation
		const auto rotMat = glm::rotate(-(joystickAngle + glm::half_pi<float>()), GetNodeUpWCS());
		const glm::vec3 moveDir = rotMat * glm::vec4(GetNodeForwardWCS(), 1.f);

		applyTo->AddNodePositionOffsetLCS(moveDir * speed * gamepad.lt);
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

	if (input.IsDown(Key::Mouse_RightClick)) {
		if (input.AreKeysDown(Key::W /*, Key::GAMEPAD_DPAD_UP*/)) {
			applyTo->AddNodePositionOffsetLCS(forward * speed);
		}

		if (input.AreKeysDown(Key::S /*, Key::GAMEPAD_DPAD_DOWN*/)) {
			applyTo->AddNodePositionOffsetLCS((-forward) * speed);
		}

		if (input.AreKeysDown(Key::D /*, Key::GAMEPAD_DPAD_RIGHT*/)) {
			applyTo->AddNodePositionOffsetLCS(right * speed);
		}

		if (input.AreKeysDown(Key::A /*, Key::GAMEPAD_DPAD_LEFT*/)) {
			applyTo->AddNodePositionOffsetLCS((-right) * speed);
		}

		if (input.AreKeysDown(Key::E /*, Key::GAMEPAD_RIGHT_SHOULDER*/)) {
			applyTo->AddNodePositionOffsetLCS(up * speed);
		}

		if (input.AreKeysDown(Key::Q /*, Key::GAMEPAD_LEFT_SHOULDER*/)) {
			applyTo->AddNodePositionOffsetLCS((-up) * speed);
		}
	}
}

void EditorCameraNode::ResetRotation()
{
	auto pyr = GetNodeEulerAnglesWCS();
	SetNodeOrientationWCS(glm::identity<glm::quat>());
	RotateNodeAroundAxisWCS(Universe::MainWorld->GetRoot()->GetNodeUpWCS(), pyr.y);
}
