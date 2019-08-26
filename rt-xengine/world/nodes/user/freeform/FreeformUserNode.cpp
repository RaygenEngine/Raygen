#include "pch.h"

#include "world/nodes/user/freeform/FreeformUserNode.h"
#include "assets/other/xml/ParsingAux.h"

namespace World
{
	FreeformUserNode::FreeformUserNode(Node* parent)
		: UserNode(parent),
		  m_camera(nullptr)
	{
	}

	std::string FreeformUserNode::ToString(bool verbose, uint depth) const
	{
		return std::string("    ") * depth + "|--FreeformUser " + Node::ToString(verbose, depth);
	}

	bool FreeformUserNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
	{
		UserNode::LoadAttributesFromXML(xmlData);

		glm::vec3 localLookat{};
		if (Assets::ReadFloatsAttribute(xmlData, "lookat", localLookat))
		{
			// if lookat read overwrite following
			SetLocalOrientation(Core::GetOrientationFromLookAtAndPosition(localLookat, GetLocalTranslation()));
		}

		return true;
	}

	bool FreeformUserNode::LoadChildrenFromXML(const tinyxml2::XMLElement* xmlData)
	{
		// children
		for (auto* xmdChildElement = xmlData->FirstChildElement(); xmdChildElement != nullptr;
			xmdChildElement = xmdChildElement->NextSiblingElement())
		{
			const std::string type = xmdChildElement->Name();
			if (type == "camera")
			{
				m_camera = GetWorld()->LoadNode<CameraNode>(this, xmdChildElement);
			}
		}

		// if head loaded successfully
		return m_camera != nullptr;
	}

	// TODO: speed and turning speed adjustments
	void FreeformUserNode::Update()
	{
		auto& input = GetInput();

		auto speed = m_movementSpeed; // 0,01

		// user buffs
		if (input.IsKeyRepeat(XVK_LSHIFT))
			speed *= 10.f;
		if (input.IsKeyRepeat(XVK_C))
			speed /= 10.f;
		if (input.IsRightTriggerMoving())
			speed *= 10.f * glm::exp(input.GetRightTriggerMagnitude());
		if (input.IsLeftTriggerMoving())
			speed /= 10.f * glm::exp(input.GetLeftTriggerMagnitude());

		// user rotation
		if (input.IsCursorDragged() && input.IsKeyRepeat(XVK_RBUTTON))
		{
			const float yaw = -input.GetCursorRelativePosition().x * m_turningSpeed;
			const float pitch = -input.GetCursorRelativePosition().y * m_turningSpeed;

			OrientWithoutRoll(yaw, pitch);
		}

		if (input.IsRightThumbMoving())
		{
			const auto yaw = -input.GetRightThumbDirection().x * input.GetRightThumbMagnitude() * 2.5f * m_turningSpeed;
			// upside down with regards to the cursor dragging
			const auto pitch = input.GetRightThumbDirection().y * input.GetRightThumbMagnitude() * 2.5f * m_turningSpeed;

			OrientWithoutRoll(yaw, pitch);
		}

		// user movement
		if (input.IsLeftThumbMoving())
		{
			//Calculate angle
			const float joystickAngle = atan2(-input.GetLeftThumbDirection().y * input.GetLeftThumbMagnitude(),
				 input.GetLeftThumbDirection().x * input.GetLeftThumbMagnitude());

			// adjust angle to match user rotation
			const auto rotMat = glm::rotate(-(joystickAngle + glm::half_pi<float>()), GetUp());
			const glm::vec3 moveDir = rotMat * glm::vec4(GetFront(), 1.f);

			Move(moveDir, speed * input.GetLeftThumbMagnitude());
		}

		if (input.IsAnyOfKeysRepeat(XVK_W, XVK_GAMEPAD_DPAD_UP))
			MoveFront(speed);

		if (input.IsAnyOfKeysRepeat(XVK_S, XVK_GAMEPAD_DPAD_DOWN))
			MoveBack(speed);

		if (input.IsAnyOfKeysRepeat(XVK_D, XVK_GAMEPAD_DPAD_RIGHT))
			MoveRight(speed);

		if (input.IsAnyOfKeysRepeat(XVK_A, XVK_GAMEPAD_DPAD_LEFT))
			MoveLeft(speed);

		if (input.IsAnyOfKeysRepeat(XVK_PAGEUP, XVK_GAMEPAD_LEFT_SHOULDER))
			MoveUp(speed);

		if (input.IsAnyOfKeysRepeat(XVK_PAGEDOWN, XVK_GAMEPAD_RIGHT_SHOULDER))
			MoveDown(speed);
	}
}
