#pragma once

#include "world/nodes/Node.h"


class LightNode : public Node
{
	REFLECTED_NODE(LightNode, Node)
	{
		REFLECT_VAR(m_near);
		REFLECT_VAR(m_far);
		REFLECT_VAR(m_color, PropertyFlags::Color);
		REFLECT_VAR(m_intensity);
		REFLECT_VAR(m_shadowMapWidth);
		REFLECT_VAR(m_shadowMapHeight);
	}

	glm::vec3 m_color;
	float m_intensity;

	int32 m_shadowMapWidth{ 2048 };
	int32 m_shadowMapHeight{ 2048 };

public:
	// TODO:
	float m_near{ 1.f };
	float m_far{ 100.5f };
	
	LightNode(Node* parent);
	~LightNode() = default;

	[[nodiscard]] glm::vec3 GetColor() const { return m_color; }
	[[nodiscard]] float GetIntensity() const { return m_intensity; }
	[[nodiscard]] int32 GetShadowMapWidth() const { return m_shadowMapWidth; }
	[[nodiscard]] int32 GetShadowMapHeight() const { return m_shadowMapHeight; }

	std::string ToString(bool verbose, uint depth) const override;

	void ToString(std::ostream& os) const override { os << "node-type: LightNode, name: " << m_name; }

	
};
