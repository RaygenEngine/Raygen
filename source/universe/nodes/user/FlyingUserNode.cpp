#include "pch.h"
#include "FlyingUserNode.h"

#include "asset/util/ParsingUtl.h"
#include "engine/Engine.h"
#include "engine/Input.h"
#include "universe/NodeFactory.h"
#include "universe/nodes/camera/WindowCameraNode.h"
#include "universe/World.h"

void FlyingUserNode::Update(float deltaTime)
{
	UserNode::Update(deltaTime);

	auto& input = Engine.GetInput();
	auto& gamepad = input.GetGamepadState();

	float pitch = 0.f;
	float yaw = 0.f;
	float roll = 0.f;
	float forward = m_baseSpeed * deltaTime;

	if (input.IsMouseDragging()) {
		yaw += -input.GetMouseDelta().x * m_inputYawMultiplier * 0.1f;
		pitch += -input.GetMouseDelta().y * m_inputPitchMultiplier * 0.1f;
	}

	if (!gamepad.ls.IsResting()) {
		yaw += -gamepad.ls.GetXAxisValue(m_inputYawMultiplier) * deltaTime * m_inputSensitivity;
		pitch += -gamepad.ls.GetYAxisValue(m_inputPitchMultiplier) * deltaTime * m_inputSensitivity;
	}

	if (!gamepad.IsLTResting()) {
		roll += -gamepad.lt * m_inputSpeedMultiplier * deltaTime * m_inputSensitivity;
	}

	if (!gamepad.IsRTResting()) {
		roll += gamepad.rt * m_inputSpeedMultiplier * deltaTime * m_inputSensitivity;
	}

	// if (input.IsJustPressed(Key::GAMEPAD_Y)) {
	//	m_baseSpeed += 5.f;
	//}

	// if (input.IsJustPressed(Key::GAMEPAD_X)) {
	//	m_baseSpeed -= 5.f;
	//}

	RotateNodeAroundAxisWCS(GetNodeUpWCS(), yaw);
	RotateNodeAroundAxisWCS(GetNodeRightWCS(), pitch);
	RotateNodeAroundAxisWCS(GetNodeForwardWCS(), roll);

	AddNodePositionOffsetWCS(GetNodeForwardWCS() * forward);


	if (!gamepad.ls.IsResting()) {
		yaw += -gamepad.ls.GetXAxisValue(90.f);
		pitch += -gamepad.ls.GetYAxisValue(90.f);
	}

	if (m_handleHead) {

		auto headYaw = -gamepad.rs.GetXAxisValue() * 90.f;
		auto headPitch = gamepad.rs.GetYAxisValue() * 90.f;

		m_pilotHead->SetNodeEulerAnglesLCS(glm::vec3(headPitch, headYaw, 0.f));
	}
}

void FlyingUserNode::DirtyUpdate(DirtyFlagset flags)
{
	if (flags[DF::Children]) {
		m_pilotHead = GetOrCreateChild<WindowCameraNode>("Pilot Head");
	}
}
