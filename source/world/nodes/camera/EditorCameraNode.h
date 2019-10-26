#pragma once

#include "world/nodes/Node.h"
#include "world/nodes/camera/WindowCameraNode.h"

// ONLY USE FOR EDITOR PURPOSES
class EditorCameraNode : public WindowCameraNode {
	REFLECTED_NODE(EditorCameraNode, WindowCameraNode) {}

	float m_movementSpeed{ 1.5f };
	float m_turningSpeed{ 0.3f };
	bool m_worldAlign{ false };

protected:
	void WindowResize(int32 x, int32 y) override;

public:
	void UpdateFromEditor(float deltaTime);

	void ResetRotation();

	EditorCameraNode();
};
