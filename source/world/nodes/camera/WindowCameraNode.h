#pragma once

#include "world/nodes/Node.h"
#include "world/nodes/camera/CameraNode.h"
#include "engine/reflection/ReflectionDb.h"
#include "engine/Events.h"

class WindowCameraNode : public CameraNode {
	REFLECTED_NODE(WindowCameraNode, CameraNode) {}

public:
	WindowCameraNode();


protected:
	virtual void WindowResize(int32 width, int32 height);

	void OnWindowResizeEvent(int32 width, int32 height);
	void OnViewportUpdatedEvent();
};
