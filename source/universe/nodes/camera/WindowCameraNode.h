#pragma once
#include "engine/Events.h"
#include "engine/reflection/ReflectionDb.h"
#include "universe/nodes/camera/CameraNode.h"
#include "universe/nodes/Node.h"

class WindowCameraNode : public CameraNode {
	REFLECTED_NODE(WindowCameraNode, CameraNode) {}

public:
	WindowCameraNode();


protected:
	virtual void WindowResize(int32 width, int32 height);

	void OnWindowResizeEvent(int32 width, int32 height);
	void OnViewportUpdatedEvent();
};
