#pragma once

#include "world/nodes/Node.h"
#include "world/nodes/camera/CameraNode.h"
#include "system/EngineEvents.h"

class WindowCameraNode : public CameraNode {
public:
	DECLARE_EVENT_LISTENER(m_resizeListener, Event::OnWindowResize);

	WindowCameraNode(Node* parent);

private:
	void WindowResize(int32 width, int32 height);
};
