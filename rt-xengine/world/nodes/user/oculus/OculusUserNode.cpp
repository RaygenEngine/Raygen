#include "pch.h"
#include "OculusUserNode.h"
#include "world/World.h"


namespace World
{

	OculusUserNode::OculusUserNode(Node* parent)
		: UserNode(parent),
		  m_navigationMode(ONM_BODY_MOVEMENT_BASED_ON_HEAD_ORIENTATION),
		  m_head(nullptr)
	{
	}

	std::string OculusUserNode::ToString(bool verbose, uint depth) const
	{
		return std::string("    ") * depth + "|--OculusUser " + Node::ToString(verbose, depth);
	}

	bool OculusUserNode::LoadAttributesFromXML(const tinyxml2::XMLElement * xmlData)
	{
		UserNode::LoadAttributesFromXML(xmlData);

		glm::vec3 localLookat{};
		if (Assets::ReadFloatsAttribute(xmlData, "lookat", localLookat))
		{
			// if lookat read overwrite following
			SetLocalOrientation(Core::GetOrientationFromLookAtAndPosition(localLookat, GetLocalTranslation()));
		}

		std::string navigation{};
		Assets::ReadStringAttribute(xmlData, "navigation", navigation);

		if (Core::CaseInsensitiveCompare(navigation, "body_based"))
			m_navigationMode = ONM_BODY_MOVEMENT_BASED_ON_BODY_ORIENTATION;
		// else default is head

		return true;
	}
	bool OculusUserNode::LoadChildrenFromXML(const tinyxml2::XMLElement * xmlData)
	{
		// children
		for (auto* xmdChildElement = xmlData->FirstChildElement(); xmdChildElement != nullptr;
			xmdChildElement = xmdChildElement->NextSiblingElement())
		{
			const std::string type = xmdChildElement->Name();
			if (type == "head")
			{
				m_head = GetWorld()->LoadNode<OculusHeadNode>(this, xmdChildElement);
			}
		}

		// if head loaded successfully
		return m_head != nullptr;
	}

	//void OculusUserNode::OnUpdate(const CTickEvent& event)
	//{
	//	//auto& keyboard = GetKeyboard();
	//	//auto& mouse = GetMouse();

	//	//auto& gameController = GetGameController();

	//	//// switch modes
	//	//if (keyboard.IsKeyPressed(XVK_b))
	//	//{
	//	//	uint32 m_curr = m_navigationMode;
	//	//	++m_curr %= ONM_COUNT;
	//	//	RT_XENGINE_LOG_INFO("Switch oculus user navigation mode");
	//	//	m_navigationMode = static_cast<OCULUS_NAVIGATION_MODE>(m_curr);
	//	//}

	//	//if (keyboard.IsKeyRepeat(XVK_LSHIFT))
	//	//	m_movementSpeed = 0.1f;
	//	//else if (keyboard.IsKeyRepeat(XVK_LCTRL))
	//	//	m_movementSpeed = 0.001f;
	//	//else if (keyboard.IsKeyReleased(XVK_LSHIFT) || keyboard.IsKeyReleased(XVK_LCTRL))
	//	//	m_movementSpeed = 0.01f;
	//	//else if (!gameController.analogTrigger[Input::AS_RIGHT].rest)
	//	//	m_movementSpeed = 0.1f * gameController.analogTrigger[Input::AS_RIGHT].val;
	//	//else if (!gameController.analogTrigger[Input::AS_LEFT].rest)
	//	//	m_movementSpeed = 0.001f / (gameController.analogTrigger[Input::AS_LEFT].val + 1);
	//	//else if (gameController.analogTrigger[Input::AS_RIGHT].rest && gameController.analogTrigger[Input::AS_LEFT].rest)
	//	//	m_movementSpeed = 0.01f;

	//	//// camera rotation
	//	//if (mouse.dragged && mouse.IsButtonRepeat(XMB_RIGHT))
	//	//{
	//	//	const float yaw = -mouse.m_cursorRelativePosition.x * m_turningSpeed * GetWorld()->GetDeltaTime();
	//	//	const float pitch = -mouse.m_cursorRelativePosition.y * m_turningSpeed * GetWorld()->GetDeltaTime();

	//	//	switch (m_navigationMode)
	//	//	{
	//	//		case ONM_BODY_MOVEMENT_BASED_ON_BODY_ORIENTATION:
	//	//			OrientWithoutRoll(yaw, pitch);
	//	//			break;

	//	//		case ONM_BODY_MOVEMENT_BASED_ON_HEAD_ORIENTATION:
	//	//		default:
	//	//			OrientYaw(yaw);
	//	//			break;
	//	//	}
	//	//}

	//	//if (!gameController.analogStick[Input::AS_RIGHT].rest)
	//	//{
	//	//	const auto yaw = static_cast<float>(-gameController.analogStick[Input::AS_RIGHT].xDir * gameController.analogStick[Input::AS_RIGHT].xVal * 2.4 * m_turningSpeed * GetWorld()->GetDeltaTime());
	//	//	const auto pitch = static_cast<float>(-gameController.analogStick[Input::AS_RIGHT].yDir * gameController.analogStick[Input::AS_RIGHT].yVal * 2.4 * m_turningSpeed * GetWorld()->GetDeltaTime());

	//	//	switch (m_navigationMode)
	//	//	{
	//	//	case ONM_BODY_MOVEMENT_BASED_ON_BODY_ORIENTATION:
	//	//		OrientWithoutRoll(yaw, pitch);
	//	//		break;

	//	//	case ONM_BODY_MOVEMENT_BASED_ON_HEAD_ORIENTATION:
	//	//	default:
	//	//		OrientYaw(yaw);
	//	//		break;
	//	//	}
	//	//}

	//	//glm::vec3 front, right, up;
	//	//switch(m_navigationMode)
	//	//{
	//	//	case ONM_BODY_MOVEMENT_BASED_ON_BODY_ORIENTATION:

	//	//		front = GetFront();
	//	//		right = GetRight();
	//	//		up = GetUp();
	//	//	
	//	//	break;

	//	//	case ONM_BODY_MOVEMENT_BASED_ON_HEAD_ORIENTATION:
	//	//	default:

	//	//		front = m_head->GetFront();
	//	//		right = m_head->GetRight();
	//	//		up = m_head->GetUp();

	//	//		break;
	//	//	
	//	//}

	//	//if (!gameController.analogStick[Input::AS_LEFT].rest)
	//	//{
	//	//	//Calculate angle
	//	//	const float joystickAngle = atan2(gameController.analogStick[Input::AS_LEFT].yDir * gameController.analogStick[Input::AS_LEFT].yVal,
	//	//		gameController.analogStick[Input::AS_LEFT].xDir* gameController.analogStick[Input::AS_LEFT].xVal);

	//	//	// adjust angle to match user rotation
	//	//	const auto rotMat = glm::rotate(-(joystickAngle + glm::half_pi<float>()), up);
	//	//	const glm::vec3 moveDir = rotMat * glm::vec4(front, 1.f);

	//	//	Move(moveDir, m_movementSpeed * GetWorld()->GetDeltaTime());
	//	//}

	//	//if (keyboard.IsKeyRepeat(XVK_w) || gameController.IsButtonRepeat(XGCB_DPAD_UP))
	//	//	Move(front, m_movementSpeed * GetWorld()->GetDeltaTime());

	//	//if (keyboard.IsKeyRepeat(XVK_s) || gameController.IsButtonRepeat(XGCB_DPAD_DOWN))
	//	//	Move(-front, m_movementSpeed * GetWorld()->GetDeltaTime());

	//	//if (keyboard.IsKeyRepeat(XVK_d) || gameController.IsButtonRepeat(XGCB_DPAD_RIGHT))
	//	//	Move(right, m_movementSpeed * GetWorld()->GetDeltaTime());

	//	//if (keyboard.IsKeyRepeat(XVK_a) || gameController.IsButtonRepeat(XGCB_DPAD_LEFT))
	//	//	Move(-right, m_movementSpeed * GetWorld()->GetDeltaTime());

	//	//if (keyboard.IsKeyRepeat(XVK_PAGEUP) || gameController.IsButtonRepeat(XGCB_LEFTSHOULDER))
	//	//	Move(up, m_movementSpeed * GetWorld()->GetDeltaTime());

	//	//if (keyboard.IsKeyRepeat(XVK_PAGEDOWN) || gameController.IsButtonRepeat(XGCB_RIGHTSHOULDER))
	//	//	Move(-up, m_movementSpeed * GetWorld()->GetDeltaTime());
	//}

	//void OculusUserNode::DispatchEvent(const EngineEvent& event)
	//{	
	//	m_head->DispatchEvent(event);
	//	EventListener::DispatchEvent(event);
	//}
}
