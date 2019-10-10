#pragma once

#include "world/nodes/Node.h"

class DirectionalLightNode : public Node {

	REFLECTED_NODE(DirectionalLightNode, Node, DF_FLAGS(Projection, Flux, ToggleShadows, ResizeShadows))
	{
		REFLECT_VAR(m_color, PropertyFlags::Color).OnDirty(DF::Flux);
		REFLECT_VAR(m_intensity).OnDirty(DF::Flux);

		REFLECT_VAR(m_hasShadow).OnDirty(DF::ToggleShadows);

		REFLECT_VAR(m_left).OnDirty(DF::Projection);
		REFLECT_VAR(m_right).OnDirty(DF::Projection);
		REFLECT_VAR(m_bottom).OnDirty(DF::Projection);
		REFLECT_VAR(m_top).OnDirty(DF::Projection);
		REFLECT_VAR(m_near).OnDirty(DF::Projection);
		REFLECT_VAR(m_far).OnDirty(DF::Projection);

		REFLECT_VAR(m_shadowMapWidth).OnDirty(DF::ResizeShadows);
		REFLECT_VAR(m_shadowMapHeight).OnDirty(DF::ResizeShadows);
	}

	glm::mat4 m_projectionMatrix{};

	float m_left{ -10.f };
	float m_right{ 10.f };

	float m_bottom{ -10.f };
	float m_top{ 10.f };

	glm::vec3 m_color{ glm::vec3(1.f) };
	float m_intensity{ 10.f };

	bool m_hasShadow{ true };

	int32 m_shadowMapWidth{ 2048 };
	int32 m_shadowMapHeight{ 2048 };

	float m_near{ 1.f };
	float m_far{ 15.5f };

public:
	DirectionalLightNode(Node* parent)
		: Node(parent)
	{
	}

	void DirtyUpdate(DirtyFlagset flags) override;

	void UpdateProjectionMatrix() { m_projectionMatrix = glm::ortho(m_left, m_right, m_bottom, m_top, m_near, m_far); }
	[[nodiscard]] glm::mat4 GetProjectionMatrix() const { return m_projectionMatrix; }

	[[nodiscard]] float GetOrthoFrustumLeft() const { return m_left; }
	[[nodiscard]] float GetOrthoFrustumRight() const { return m_right; }
	[[nodiscard]] float GetOrthoFrustumBottom() const { return m_bottom; }
	[[nodiscard]] float GetOrthoFrustumTop() const { return m_top; }
	[[nodiscard]] glm::vec3 GetColor() const { return m_color; }
	[[nodiscard]] float GetIntensity() const { return m_intensity; }
	[[nodiscard]] int32 GetShadowMapWidth() const { return m_shadowMapWidth; }
	[[nodiscard]] int32 GetShadowMapHeight() const { return m_shadowMapHeight; }
	[[nodiscard]] float GetNear() const { return m_near; }
	[[nodiscard]] float GetFar() const { return m_far; }
	[[nodiscard]] bool CastsShadows() const { return m_hasShadow; }
};
