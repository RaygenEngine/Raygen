#include "pch/pch.h"

#include "world/nodes/user/freeform/FreeformUserNode.h"
#include "world/World.h"
#include "asset/util/ParsingAux.h"
#include "system/Engine.h"
#include "system/Input.h"


FreeformUserNode::FreeformUserNode(Node* parent)
	: UserNode(parent)
{
}

// TODO: speed and turning speed adjustments
void FreeformUserNode::Update(float deltaTime)
{
	auto& input = *Engine::GetInput();

	m_movementSpeed = glm::clamp(m_movementSpeed + input.GetWheelDelta() * 2.0f, 10.f, 100.0f);
	auto speed = m_movementSpeed;

	speed *= deltaTime;

	// user buffs
	if (input.IsKeyRepeat(XVirtualKey::LSHIFT))
		speed *= 10.f;
	if (input.IsKeyRepeat(XVirtualKey::CTRL))
		speed /= 10.f;
	if (input.IsRightTriggerMoving())
		speed *= 10.f * glm::exp(input.GetRightTriggerMagnitude());
	if (input.IsLeftTriggerMoving())
		speed /= 10.f * glm::exp(input.GetLeftTriggerMagnitude());

	// user rotation
	if (input.IsCursorDragged() && input.IsKeyRepeat(XVirtualKey::RBUTTON)) {
		const float yaw = -input.GetCursorRelativePosition().x * m_turningSpeed * 0.005f;
		const float pitch = -input.GetCursorRelativePosition().y * m_turningSpeed * 0.005f;

		OrientWithoutRoll(yaw, pitch);
	}

	if (input.IsRightThumbMoving()) {
		const auto yaw
			= -input.GetRightThumbDirection().x * input.GetRightThumbMagnitude() * 2.5f * m_turningSpeed * deltaTime;
		// upside down with regards to the cursor dragging
		const auto pitch
			= input.GetRightThumbDirection().y * input.GetRightThumbMagnitude() * 2.5f * m_turningSpeed * deltaTime;

		OrientWithoutRoll(yaw, pitch);
	}

	// user movement
	if (input.IsLeftThumbMoving()) {
		// Calculate angle
		const float joystickAngle = atan2(-input.GetLeftThumbDirection().y * input.GetLeftThumbMagnitude(),
			input.GetLeftThumbDirection().x * input.GetLeftThumbMagnitude());

		// adjust angle to match user rotation
		const auto rotMat = glm::rotate(-(joystickAngle + glm::half_pi<float>()), GetUp());
		const glm::vec3 moveDir = rotMat * glm::vec4(GetFront(), 1.f);

		AddLocalOffset(moveDir * speed * input.GetLeftThumbMagnitude());
	}

	if (input.IsAnyOfKeysRepeat(XVirtualKey::W, XVirtualKey::GAMEPAD_DPAD_UP))
		AddLocalOffset(GetFront() * speed);

	if (input.IsAnyOfKeysRepeat(XVirtualKey::S, XVirtualKey::GAMEPAD_DPAD_DOWN))
		AddLocalOffset((-GetFront()) * speed);

	if (input.IsAnyOfKeysRepeat(XVirtualKey::D, XVirtualKey::GAMEPAD_DPAD_RIGHT))
		AddLocalOffset((GetRight()) * speed);

	if (input.IsAnyOfKeysRepeat(XVirtualKey::A, XVirtualKey::GAMEPAD_DPAD_LEFT))
		AddLocalOffset((-GetRight()) * speed);

	if (input.IsAnyOfKeysRepeat(XVirtualKey::E, XVirtualKey::GAMEPAD_LEFT_SHOULDER))
		AddLocalOffset((GetUp()) * speed);

	if (input.IsAnyOfKeysRepeat(XVirtualKey::Q, XVirtualKey::GAMEPAD_RIGHT_SHOULDER))
		AddLocalOffset((-GetUp()) * speed);
}
