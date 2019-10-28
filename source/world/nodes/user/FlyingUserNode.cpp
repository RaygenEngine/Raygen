#include "pch/pch.h"

#include "world/nodes/user/FlyingUserNode.h"
#include "world/nodes/camera/WindowCameraNode.h"
#include "world/NodeFactory.h"
#include "asset/util/ParsingAux.h"
#include "world/World.h"
#include "system/Engine.h"
#include "system/Input.h"

void FlyingUserNode::Update(float deltaTime)
{
	UserNode::Update(deltaTime);

	auto& input = *Engine::GetInput();
	auto& analog = input.GetAnalogState();

	float pitch = 0.f;
	float yaw = 0.f;
	float roll = 0.f;
	float forward = m_baseSpeed * deltaTime;

	if (input.IsCursorDragged() && input.IsKeyRepeat(Key::RBUTTON)) {
		yaw += -input.GetCursorRelativePosition().x * m_inputYawMultiplier * 0.1f;
		pitch += -input.GetCursorRelativePosition().y * m_inputPitchMultiplier * 0.1f;
	}

	if (!input.IsLeftThumbResting()) {
		yaw += -analog.ls.GetXAxisValue(m_inputYawMultiplier) * deltaTime * m_inputSensitivity;
		pitch += -analog.ls.GetYAxisValue(m_inputPitchMultiplier) * deltaTime * m_inputSensitivity;
	}

	if (!input.IsLeftTriggerResting()) {
		roll += -analog.lt * m_inputSpeedMultiplier * deltaTime * m_inputSensitivity;
	}

	if (!input.IsRightTriggerResting()) {
		roll += analog.rt * m_inputSpeedMultiplier * deltaTime * m_inputSensitivity;
	}


	if (input.IsKeyPressed(Key::GAMEPAD_Y)) {
		m_baseSpeed += 5.f;
	}

	if (input.IsKeyPressed(Key::GAMEPAD_X)) {
		m_baseSpeed -= 5.f;
	}


	RotateAroundAxis(GetUp(), yaw);
	RotateAroundAxis(GetRight(), pitch);
	RotateAroundAxis(GetForward(), roll);

	AddOffset(GetForward() * forward);


	if (!input.IsRightThumbResting()) {
		yaw += -analog.ls.GetXAxisValue(90.f);
		pitch += -analog.ls.GetYAxisValue(90.f);
	}

	if (m_handleHead) {

		auto headYaw = -analog.rs.GetXAxisValue() * 90.f;
		auto headPitch = analog.rs.GetYAxisValue() * 90.f;

		m_pilotHead->SetLocalPYR(glm::vec3(headPitch, headYaw, 0.f));
	}
}

void FlyingUserNode::DirtyUpdate(DirtyFlagset flags)
{
	if (flags[DF::Children]) {
		m_pilotHead = GetOrCreateChild<WindowCameraNode>("Pilot Head");
	}
}
