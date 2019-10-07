#pragma once

#include "world/nodes/Node.h"


class LightNode : public Node
{
	REFLECTED_NODE(LightNode, Node)
	{
		REFLECT_VAR(m_hasShadow);
		REFLECT_VAR(m_color, PropertyFlags::Color);
		REFLECT_VAR(m_intensity);

		REFLECT_VAR(m_shadowMapWidth);
		REFLECT_VAR(m_shadowMapHeight);
		REFLECT_VAR(m_near);
		REFLECT_VAR(m_far);
	}

	DECLARE_DIRTY_FLAGSET(
		ShadowMapSize,
		ColorAndIntensitiy,
		NearAndFar,
		HasShadowChanged
	)

	glm::vec3 m_color;
	float m_intensity;

	bool m_hasShadow;

protected:
	
	int32 m_shadowMapWidth{ 2048 };
	int32 m_shadowMapHeight{ 2048 };

	float m_near{ 1.f };
	float m_far{ 100.5f };

public:

	LightNode(Node* parent);
	~LightNode() = default;

	[[nodiscard]] glm::vec3 GetColor() const { return m_color; }
	[[nodiscard]] float GetIntensity() const { return m_intensity; }
	[[nodiscard]] int32 GetShadowMapWidth() const { return m_shadowMapWidth; }
	[[nodiscard]] int32 GetShadowMapHeight() const { return m_shadowMapHeight; }
	[[nodiscard]] float GetNear() const { return m_near; }
	[[nodiscard]] float GetFar() const { return m_far; }
	

	virtual void DirtyUpdate();
};
