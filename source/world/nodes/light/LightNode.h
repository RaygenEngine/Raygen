#pragma once

#include "world/nodes/Node.h"


class LightNode : public Node
{
	REFLECTED_NODE(LightNode, Node)
	{
		REFLECT_VAR(m_color, PropertyFlags::Color);
		REFLECT_VAR(m_intensity);
	}

	// TODO: should this be a flux vec3
	glm::vec3 m_color;
	float m_intensity;

public:
	LightNode(Node* parent);
	~LightNode() = default;

	[[nodiscard]] glm::vec3 GetColor() const { return m_color; }
	[[nodiscard]] float GetIntensity() const { return m_intensity; }

	std::string ToString(bool verbose, uint depth) const override;

	void ToString(std::ostream& os) const override { os << "node-type: LightNode, name: " << m_name; }
};
