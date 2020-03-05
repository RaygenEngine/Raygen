#include "pch.h"

#include "world/nodes/user/FreeformUserNode.h"
#include "world/nodes/RootNode.h"
#include "asset/util/ParsingAux.h"
#include "system/profiler/ProfileScope.h"
#include "system/Engine.h"
#include "system/Input.h"

void FreeformUserNode::Update(float deltaTime)
{
	PROFILE_SCOPE(World);
	auto& input = Engine::GetInput();
	auto& gamepad = input.GetGamepadState();

	m_movementSpeed = glm::clamp(m_movementSpeed + input.GetScrollDelta() / 240.f * 2.0f, 1.0f, 100.0f);
	auto speed = m_movementSpeed;

	speed *= deltaTime;

	if (!gamepad.IsRTResting()) {
		speed *= 10.f * glm::exp(gamepad.rt);
	}

	if (!gamepad.IsLTResting()) {
		speed /= 10.f * glm::exp(gamepad.lt);
	}

	// user rotation
	if (input.IsMouseDragging()) {
		const float yaw = -input.GetMouseDelta().x * m_turningSpeed * 0.5f;
		const float pitch = -input.GetMouseDelta().y * m_turningSpeed * 0.5f;

		RotateNodeAroundAxisWCS(GetParent()->GetNodeUpWCS(), yaw);
		RotateNodeAroundAxisWCS(GetNodeRightWCS(), pitch);
	}

	if (!gamepad.IsRTResting()) {
		const auto yaw = -gamepad.rs.GetXAxisValue() * 2.5f * m_turningSpeed * deltaTime;
		// upside down with regards to the cursor dragging
		const auto pitch = gamepad.rs.GetYAxisValue() * 2.5f * m_turningSpeed * deltaTime;

		RotateNodeAroundAxisWCS(GetParent()->GetNodeUpWCS(), glm::radians(yaw));
		RotateNodeAroundAxisWCS(GetNodeRightWCS(), glm::radians(pitch));
	}

	// user movement
	if (!gamepad.IsLTResting()) {
		// Calculate angle
		const float joystickAngle = atan2(-gamepad.ls.GetYAxisValue(), gamepad.ls.GetXAxisValue());

		// adjust angle to match user rotation
		const auto rotMat = glm::rotate(-(joystickAngle + glm::half_pi<float>()), GetNodeUpWCS());
		const glm::vec3 moveDir = rotMat * glm::vec4(GetNodeForwardWCS(), 1.f);

		AddNodePositionOffsetLCS(moveDir * speed * gamepad.lt);
	}

	if (input.AreKeysDown(Key::W /*, Key::GAMEPAD_DPAD_UP*/)) {
		AddNodePositionOffsetLCS(GetNodeForwardLCS() * speed);
	}

	if (input.AreKeysDown(Key::S /*, Key::GAMEPAD_DPAD_DOWN*/)) {
		AddNodePositionOffsetLCS((-GetNodeForwardLCS()) * speed);
	}

	if (input.AreKeysDown(Key::D /*, Key::GAMEPAD_DPAD_RIGHT*/)) {
		AddNodePositionOffsetLCS((GetNodeRightLCS()) * speed);
	}

	if (input.AreKeysDown(Key::A /*, Key::GAMEPAD_DPAD_LEFT*/)) {
		AddNodePositionOffsetLCS((-GetNodeRightLCS()) * speed);
	}

	if (input.AreKeysDown(Key::E /*, Key::GAMEPAD_RIGHT_SHOULDER*/)) {
		AddNodePositionOffsetLCS((GetWorldRoot()->GetNodeUpWCS()) * speed);
	}

	if (input.AreKeysDown(Key::Q /*, Key::GAMEPAD_LEFT_SHOULDER*/)) {
		AddNodePositionOffsetLCS((-GetWorldRoot()->GetNodeUpWCS()) * speed);
	}
}
