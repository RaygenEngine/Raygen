#pragma once

#include "world/nodes/user/UserNode.h"

class FlyingUserNode : public UserNode {
	REFLECTED_NODE(FlyingUserNode, UserNode)
	{
		REFLECT_VAR(m_totalSpeed, PropertyFlags::Generated);
		REFLECT_VAR(m_baseSpeed);
		REFLECT_VAR(m_inputSpeedMultiplier);

		REFLECT_VAR(m_inputPitchMultiplier);
		REFLECT_VAR(m_inputYawMultiplier);
		REFLECT_VAR(m_inputRollMultiplier);

		REFLECT_VAR(m_inputSensitivity);


		REFLECT_VAR(m_handleHead);
	}

	float m_totalSpeed{ 0.0f };
	float m_baseSpeed{ 0.f };
	float m_inputSpeedMultiplier{ 30.f };

	float m_inputPitchMultiplier{ 45.f };
	float m_inputYawMultiplier{ 45.f };
	float m_inputRollMultiplier{ 45.f };

	float m_inputSensitivity{ 3.f };

	bool m_handleHead{ true };

	Node* m_pilotHead;

public:
	void Update(float deltaTime) override;

	void DirtyUpdate(DirtyFlagset flags) override;
};
