#include "pch/pch.h"

#include "world/nodes/user/FreeformUserNode.h"
#include "world/nodes/RootNode.h"
#include "asset/util/ParsingAux.h"
#include "system/Engine.h"
#include "system/Input.h"

void FreeformUserNode::Update(float deltaTime)
{
	auto& input = *Engine::GetInput();

	m_movementSpeed = glm::clamp(m_movementSpeed + input.GetWheelDelta() / 240.f * 2.0f, 1.0f, 100.0f);
	auto speed = m_movementSpeed;

	speed *= deltaTime;

	if (!input.IsRightTriggerResting()) {
		speed *= 10.f * glm::exp(input.GetRightTriggerMagnitude());
	}

	if (!input.IsLeftTriggerResting()) {
		speed /= 10.f * glm::exp(input.GetLeftTriggerMagnitude());
	}

	// user rotation
	if (input.IsCursorDragged() && input.IsKeyRepeat(Key::RBUTTON)) {
		const float yaw = -input.GetCursorRelativePosition().x * m_turningSpeed * 0.5f;
		const float pitch = -input.GetCursorRelativePosition().y * m_turningSpeed * 0.5f;

		RotateNodeAroundAxisWCS(GetParent()->GetNodeUpWCS(), yaw);
		RotateNodeAroundAxisWCS(GetNodeRightWCS(), pitch);
	}

	if (!input.IsRightThumbResting()) {
		const auto yaw
			= -input.GetRightThumbDirection().x * input.GetRightThumbMagnitude() * 2.5f * m_turningSpeed * deltaTime;
		// upside down with regards to the cursor dragging
		const auto pitch
			= input.GetRightThumbDirection().y * input.GetRightThumbMagnitude() * 2.5f * m_turningSpeed * deltaTime;

		RotateNodeAroundAxisWCS(GetParent()->GetNodeUpWCS(), glm::radians(yaw));
		RotateNodeAroundAxisWCS(GetNodeRightWCS(), glm::radians(pitch));
	}

	// user movement
	if (!input.IsLeftThumbResting()) {
		// Calculate angle
		const float joystickAngle = atan2(-input.GetLeftThumbDirection().y * input.GetLeftThumbMagnitude(),
			input.GetLeftThumbDirection().x * input.GetLeftThumbMagnitude());

		// adjust angle to match user rotation
		const auto rotMat = glm::rotate(-(joystickAngle + glm::half_pi<float>()), GetNodeUpWCS());
		const glm::vec3 moveDir = rotMat * glm::vec4(GetNodeForwardWCS(), 1.f);

		AddNodePositionOffsetLCS(moveDir * speed * input.GetLeftThumbMagnitude());
	}

	if (input.IsAnyOfKeysRepeat(Key::W, Key::GAMEPAD_DPAD_UP)) {
		AddNodePositionOffsetLCS(GetNodeForwardLCS() * speed);
	}

	if (input.IsAnyOfKeysRepeat(Key::S, Key::GAMEPAD_DPAD_DOWN)) {
		AddNodePositionOffsetLCS((-GetNodeForwardLCS()) * speed);
	}

	if (input.IsAnyOfKeysRepeat(Key::D, Key::GAMEPAD_DPAD_RIGHT)) {
		AddNodePositionOffsetLCS((GetNodeRightLCS()) * speed);
	}

	if (input.IsAnyOfKeysRepeat(Key::A, Key::GAMEPAD_DPAD_LEFT)) {
		AddNodePositionOffsetLCS((-GetNodeRightLCS()) * speed);
	}
	if (input.IsAnyOfKeysRepeat(Key::E, Key::GAMEPAD_LEFT_SHOULDER)) {
		AddNodePositionOffsetLCS((GetWorldRoot()->GetNodeUpWCS()) * speed);
	}

	if (input.IsAnyOfKeysRepeat(Key::Q, Key::GAMEPAD_RIGHT_SHOULDER)) {
		AddNodePositionOffsetLCS((-GetWorldRoot()->GetNodeUpWCS()) * speed);
	}
}
