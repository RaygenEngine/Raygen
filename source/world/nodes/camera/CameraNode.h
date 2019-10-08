#pragma once

#include "world/nodes/Node.h"
#include "system/EngineEvents.h"

// Note: to make a automatic camera that resizes to window size see WindowCameraNode
class CameraNode : public Node
{
	REFLECTED_NODE(CameraNode, Node)
	{
		REFLECT_VAR(m_far)			.OnDirty(DF::Projection);
		REFLECT_VAR(m_focalLength)	.OnDirty(DF::Projection);
		REFLECT_VAR(m_vFov)			.OnDirty(DF::Projection);
		REFLECT_VAR(m_hFov)			.OnDirty(DF::Projection);
		REFLECT_VAR(m_near)			.OnDirty(DF::Projection);

		REFLECT_VAR(m_viewportWidth, PropertyFlags::Transient)
			.OnDirty(DF::ViewportSize);

		REFLECT_VAR(m_viewportHeight, PropertyFlags::Transient)
			.OnDirty(DF::ViewportSize);
	}

	DECLARE_DIRTY_FLAGSET(Projection, ViewportSize)

protected:
	// distance to film plane
	float m_focalLength;

	// vertical fov (full angle)
	float m_vFov;
	// horizontal fov depends on the vertical and the aspect ratio
	float m_hFov;

	float m_near;
	float m_far;

	glm::mat4 m_projectionMatrix;

	int32 m_viewportWidth{ 1280 };
	int32 m_viewportHeight{ 720 };
	
public:
	CameraNode(Node* parent);
	~CameraNode() = default;

	glm::vec3 GetLookAt() const override { return GetWorldTranslation() + GetFront() * m_focalLength; }

	float GetVerticalFov() const { return m_vFov; }
	float GetHorizontalFov() const { return m_hFov; }
	float GetFocalLength() const { return m_focalLength; }

	float GetNear() const { return m_near; }
	float GetFar() const { return m_far; }

	int32 GetWidth() const { return m_viewportWidth; }
	int32 GetHeight() const { return m_viewportHeight; }

	void RecalculateProjectionFov();

	glm::mat4 GetProjectionMatrix() const { return m_projectionMatrix; }
		
	void DirtyUpdate() override;
};


