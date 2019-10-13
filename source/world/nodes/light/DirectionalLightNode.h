#pragma once

#include "world/nodes/light/LightNode.h"

class DirectionalLightNode : public LightNode {

	REFLECTED_NODE(DirectionalLightNode, LightNode, DF_FLAGS(Projection))
	{
		REFLECT_VAR(m_left).OnDirty(DF::Projection);
		REFLECT_VAR(m_right).OnDirty(DF::Projection);
		REFLECT_VAR(m_bottom).OnDirty(DF::Projection);
		REFLECT_VAR(m_top).OnDirty(DF::Projection);
	}

	glm::mat4 m_projectionMatrix{};
	glm::mat4 m_viewMatrix{};
	glm::mat4 m_viewProjectionMatrix{};

	float m_left{ -10.f };
	float m_right{ 10.f };

	float m_bottom{ -10.f };
	float m_top{ 10.f };

	void RecalculateProjectionMatrix();
	void RecalculateViewMatrix();
	void RecalculateViewProjectionMatrix();

public:
	void DirtyUpdate(DirtyFlagset flags) override;

	[[nodiscard]] float GetOrthoFrustumLeft() const { return m_left; }
	[[nodiscard]] float GetOrthoFrustumRight() const { return m_right; }
	[[nodiscard]] float GetOrthoFrustumBottom() const { return m_bottom; }
	[[nodiscard]] float GetOrthoFrustumTop() const { return m_top; }

	[[nodiscard]] glm::mat4 GetProjectionMatrix() const { return m_projectionMatrix; }
	[[nodiscard]] glm::mat4 GetViewMatrix() const { return m_viewMatrix; }
	[[nodiscard]] glm::mat4 GetViewProjectionMatrix() const { return m_viewProjectionMatrix; }
};
