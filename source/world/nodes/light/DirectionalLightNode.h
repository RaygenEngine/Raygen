#pragma once

#include "world/nodes/Node.h"
#include "world/nodes/light/LightNode.h"


class DirectionalLightNode : public LightNode
{
	REFLECTED_NODE(DirectionalLightNode, LightNode)
	{
		REFLECT_VAR(m_left, PropertyFlags::Color)
			.OnDirty(DF::Projection);
		
		REFLECT_VAR(m_right)
			.OnDirty(DF::Projection);

		REFLECT_VAR(m_bottom)
			.OnDirty(DF::Projection);
		
		REFLECT_VAR(m_top)
			.OnDirty(DF::Projection);
	}

	DECLARE_DIRTY_FLAGSET(Projection)

	glm::mat4 m_orthoProjectionMatrix;

	float m_left{ -10.f };
	float m_right{ 10.f };

	float m_bottom{ -10.f };
	float m_top{ 10.f };
	
public:

	DirectionalLightNode(Node* parent);
	~DirectionalLightNode() = default;

	bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;
	
	void DirtyUpdate() override;

	void UpdateOrthoProjectionMatrix() { m_orthoProjectionMatrix = glm::ortho(m_left, m_right, m_bottom, m_top, m_near, m_far);  }
	glm::mat4 GetOrthoProjectionMatrix() const { return m_orthoProjectionMatrix; }
};
