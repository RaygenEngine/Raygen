#pragma once

#include "world/nodes/Node.h"
#include "platform/windows/Win32Window.h"
#include "system/Engine.h"
#include "system/EngineEvents.h"

// Window dependant // TODO: non-window dependant (e.g. oculus virtual eye)
class CameraNode : public Node
{
	REFLECTED_NODE(CameraNode, Node)
	{
		REFLECT_VAR(m_far);
		REFLECT_VAR(m_focalLength);
		REFLECT_VAR(m_vFov);
		REFLECT_VAR(m_hFov);
		REFLECT_VAR(m_near);

		REFLECT_VAR(m_viewportWidth, PropertyFlags::Transient);
		REFLECT_VAR(m_viewportHeight, PropertyFlags::Transient);
	}

	DECLARE_DIRTY_FLAGSET(Projection, ViewportSize)
	
	// distance to film plane
	float m_focalLength;

	// vertical fov (full angle)
	float m_vFov;
	// horizontal fov depends on the vertical and the aspect ratio
	float m_hFov;

	float m_near;
	float m_far;

	glm::mat4 m_projectionMatrix;

	int32 m_viewportWidth;
	int32 m_viewportHeight;
	
public:
	DECLARE_EVENT_LISTENER(m_resizeListener, Event::OnWindowResize);

	CameraNode(Node* parent);
	~CameraNode() = default;

	glm::vec3 GetLookAt() const { return GetWorldTranslation() + (GetWorldOrientation() * glm::vec3(0.f, 0.f, -1.f)) * m_focalLength; }

	float GetVerticalFov() const { return m_vFov; }
	float GetHorizontalFov() const { return m_hFov; }
	float GetFocalLength() const { return m_focalLength; }

	float GetNear() const { return m_near; }
	float GetFar() const { return m_far; }

	int32 GetWidth() const { return m_viewportWidth; }
	int32 GetHeight() const { return m_viewportHeight; }

	void RecalculateProjectionFov();


	glm::mat4 GetViewMatrix() const { return glm::lookAt(GetWorldTranslation(), GetLookAt(), GetUp()); }
	glm::mat4 GetProjectionMatrix() const { return m_projectionMatrix; }
		
	virtual void DirtyUpdate() override;
private:
	void WindowResize(int32 width, int32 height);
};


