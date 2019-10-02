#pragma once

#include "world/nodes/Node.h"
#include "world/nodes/light/LightNode.h"


class DirectionalLightNode : public LightNode
{
	REFLECTED_NODE(DirectionalLightNode, LightNode)
	{
		REFLECT_VAR(m_near);
		REFLECT_VAR(m_far);

		REFLECT_VAR(m_left);
		REFLECT_VAR(m_right);

		REFLECT_VAR(m_bottom);
		REFLECT_VAR(m_top);
	}

public:
	float m_near{ 1.f };
	float m_far{ 100.5f };

	float m_left{ -10.f };
	float m_right{ 10.f };

	float m_bottom{ -10.f };
	float m_top{ 10.f };


	DirectionalLightNode(Node* parent);
	~DirectionalLightNode() = default;

	bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;
	
	std::string ToString(bool verbose, uint depth) const override;

	void ToString(std::ostream& os) const override { os << "node-type: DirectionalLightNode, name: " << m_name; }
};
