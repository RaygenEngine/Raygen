#pragma once

#include "world/nodes/Node.h"
#include "core/MathAux.h"

// Note: to make a automatic camera that resizes to window size see WindowCameraNode
class CameraNode : public Node {

	friend class OVRNode;

	REFLECTED_NODE(CameraNode, Node, DF_FLAGS(Projection, ViewportSize, FocalLength))
	{
		REFLECT_VAR(m_near).OnDirty(DF::Projection);
		REFLECT_VAR(m_far).OnDirty(DF::Projection);
		REFLECT_VAR(m_focalLength).OnDirty(DF::FocalLength);
		REFLECT_VAR(m_vFov, PropertyFlags::Rads).OnDirty(DF::Projection);
		REFLECT_VAR(m_hFov, PropertyFlags::Rads).OnDirty(DF::Projection);
		REFLECT_VAR(m_vFovOffset, PropertyFlags::Rads).OnDirty(DF::Projection);
		REFLECT_VAR(m_hFovOffset, PropertyFlags::Rads).OnDirty(DF::Projection);

		REFLECT_VAR(m_viewportWidth, PropertyFlags::Transient).OnDirty(DF::ViewportSize);

		REFLECT_VAR(m_viewportHeight, PropertyFlags::Transient).OnDirty(DF::ViewportSize);

		REFLECT_VAR(m_viewMatrix, PropertyFlags::Transient);
		REFLECT_VAR(m_projectionMatrix, PropertyFlags::Transient);
		REFLECT_VAR(m_viewProjectionMatrix, PropertyFlags::Transient);
	}


protected:
	// distance to film plane
	float m_focalLength{ 1.f };

	// vertical fov (angle)
	float m_vFov{ glm::radians(72.f) };
	// horizontal fov depends on the vertical and the aspect ratio
	float m_hFov{ glm::radians(106.f) };

	float m_near{ 0.1f };
	float m_far{ 1000.f };

	float m_vFovOffset{ 0.f };
	float m_hFovOffset{ 0.f };

	glm::mat4 m_projectionMatrix{};
	glm::mat4 m_viewMatrix{};
	glm::mat4 m_viewProjectionMatrix{};
	// world space frustum TODO
	math::Frustum m_frustum{};

	int32 m_viewportWidth{ 1280 };
	int32 m_viewportHeight{ 720 };

	void CalculateWorldAABB() override;

	void RecalculateProjectionFov();
	void RecalculateViewMatrix();
	void RecalculateViewProjectionMatrix();

public:
	[[nodiscard]] glm::vec3 GetLookAt() const { return GetNodePositionWCS() + GetNodeForwardWCS() * m_focalLength; }

	[[nodiscard]] float GetVerticalFov() const { return m_vFov; }
	[[nodiscard]] float GetHorizontalFov() const { return m_hFov; }
	[[nodiscard]] float GetFocalLength() const { return m_focalLength; }

	[[nodiscard]] float GetNear() const { return m_near; }
	[[nodiscard]] float GetFar() const { return m_far; }

	[[nodiscard]] int32 GetWidth() const { return m_viewportWidth; }
	[[nodiscard]] int32 GetHeight() const { return m_viewportHeight; }

	[[nodiscard]] glm::mat4 GetProjectionMatrix() const { return m_projectionMatrix; }
	[[nodiscard]] glm::mat4 GetViewMatrix() const { return m_viewMatrix; }
	[[nodiscard]] glm::mat4 GetViewProjectionMatrix() const { return m_viewProjectionMatrix; }

	//[[nodiscard]] math::Frustum GetFrustum() const { return m_frustum; }

	bool IsNodeInsideFrustum(Node* node) { return m_frustum.IntersectsAABB(node->GetAABB()); }

	void DirtyUpdate(DirtyFlagset flags) override;
};
