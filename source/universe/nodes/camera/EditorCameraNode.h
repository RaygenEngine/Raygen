#pragma once
#include "universe/nodes/camera/WindowCameraNode.h"
#include "universe/nodes/Node.h"

// ONLY USE FOR EDITOR PURPOSES
class EditorCameraNode : public WindowCameraNode {
	REFLECTED_NODE(EditorCameraNode, WindowCameraNode) { REFLECT_FLAGS(NodeFlags::NoUserCreated); }

	float m_movementSpeed{ 1.5f };
	float m_turningSpeed{ 0.3f };
	bool m_worldAlign{ false };

public:
	void UpdateFromEditor(float deltaTime);

	void ResetRotation();
};
