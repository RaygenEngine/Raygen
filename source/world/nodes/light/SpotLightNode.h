#pragma once

#include "world/nodes/Node.h"

class SpotLightNode : public Node {

	REFLECTED_NODE(SpotLightNode, Node)
	{
		REFLECT_VAR(m_color, PropertyFlags::Color).OnDirty(DF::Flux);
		REFLECT_VAR(m_intensity).OnDirty(DF::Flux);

		REFLECT_VAR(m_hasShadow).OnDirty(DF::ToggleShadows);

		REFLECT_VAR(m_aperture).OnDirty(DF::Projection);
		REFLECT_VAR(m_near).OnDirty(DF::Projection);
		REFLECT_VAR(m_far).OnDirty(DF::Projection);

		REFLECT_VAR(m_shadowMapWidth).OnDirty(DF::ResizeShadows);
		REFLECT_VAR(m_shadowMapHeight).OnDirty(DF::ResizeShadows);
	}

	DECLARE_DIRTY_FLAGSET(Projection, Flux, ToggleShadows, ResizeShadows)

	glm::mat4 m_projectionMatrix{};

	float m_aperture{ 45.f };

	glm::vec3 m_color{ glm::vec3(1.f) };
	float m_intensity{ 10.f };

	bool m_hasShadow{ true };

	int32 m_shadowMapWidth{ 2048 };
	int32 m_shadowMapHeight{ 2048 };

	float m_near{ 1.f };
	float m_far{ 15.5f };

	void UpdateProjectionMatrix();

public:
	SpotLightNode(Node* parent)
		: Node(parent)
	{
	}

	void DirtyUpdate(DirtyFlagset flags) override;

	[[nodiscard]] glm::mat4 GetProjectionMatrix() const { return m_projectionMatrix; }
	[[nodiscard]] float GetAperture() const { return m_aperture; }
	[[nodiscard]] glm::vec3 GetColor() const { return m_color; }
	[[nodiscard]] float GetIntensity() const { return m_intensity; }
	[[nodiscard]] int32 GetShadowMapWidth() const { return m_shadowMapWidth; }
	[[nodiscard]] int32 GetShadowMapHeight() const { return m_shadowMapHeight; }
	[[nodiscard]] float GetNear() const { return m_near; }
	[[nodiscard]] float GetFar() const { return m_far; }
	[[nodiscard]] bool CastsShadows() const { return m_hasShadow; }
};
