#pragma once

#include "world/nodes/Node.h"
#include "system/EngineEvents.h"

// Note: to make a automatic camera that resizes to window size see WindowCameraNode
class CameraNode : public Node {
	REFLECTED_NODE(CameraNode, Node, DF_FLAGS(Projection, ViewportSize))
	{
		REFLECT_VAR(m_far).OnDirty(DF::Projection);
		REFLECT_VAR(m_focalLength).OnDirty(DF::Projection);
		REFLECT_VAR(m_vFov).OnDirty(DF::Projection);
		REFLECT_VAR(m_hFov).OnDirty(DF::Projection);
		REFLECT_VAR(m_near).OnDirty(DF::Projection);

		REFLECT_VAR(m_viewportWidth, PropertyFlags::Transient).OnDirty(DF::ViewportSize);

		REFLECT_VAR(m_viewportHeight, PropertyFlags::Transient).OnDirty(DF::ViewportSize);
	}


protected:
	// distance to film plane
	float m_focalLength{ 1.f };

	// vertical fov (full angle)
	float m_vFov{ 60.f };
	// horizontal fov depends on the vertical and the aspect ratio
	float m_hFov{ 45.f };

	float m_near{ 0.2f };
	float m_far{ 1000.f };

	glm::mat4 m_projectionMatrix{};

	int32 m_viewportWidth{ 1280 };
	int32 m_viewportHeight{ 720 };

	void RecalculateProjectionFov();

public:
	[[nodiscard]] glm::vec3 GetLookAt() const override { return GetWorldTranslation() + GetFront() * m_focalLength; }

	[[nodiscard]] float GetVerticalFov() const { return m_vFov; }
	[[nodiscard]] float GetHorizontalFov() const { return m_hFov; }
	[[nodiscard]] float GetFocalLength() const { return m_focalLength; }

	[[nodiscard]] float GetNear() const { return m_near; }
	[[nodiscard]] float GetFar() const { return m_far; }

	[[nodiscard]] int32 GetWidth() const { return m_viewportWidth; }
	[[nodiscard]] int32 GetHeight() const { return m_viewportHeight; }

	[[nodiscard]] glm::mat4 GetProjectionMatrix() const { return m_projectionMatrix; }

	void DirtyUpdate(DirtyFlagset flags) override;
};
