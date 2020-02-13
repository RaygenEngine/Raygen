#pragma once

#include "world/nodes/Node.h"
#include "world/nodes/camera/CameraNode.h"
#include "system/reflection/ReflectionDb.h"
#include "system/EngineEvents.h"

class WindowCameraNode : public CameraNode {
	REFLECTED_NODE(WindowCameraNode, CameraNode) {}

public:
	DECLARE_EVENT_LISTENER(m_resizeListener, Event::OnWindowResize);
	DECLARE_EVENT_LISTENER(m_viewportListener, Event::OnViewportUpdated);

	WindowCameraNode();


protected:
	virtual void WindowResize(int32 width, int32 height);

	void OnWindowResizeEvent(int32 width, int32 height);
	void OnViewportUpdatedEvent();
};
