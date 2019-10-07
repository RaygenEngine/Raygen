#pragma once

#include "world/nodes/Node.h"
#include "world/nodes/light/LightNode.h"

class SpotLightNode : public LightNode
{
	REFLECTED_NODE(SpotLightNode, LightNode)
	{
		REFLECT_VAR(m_aperture, PropertyFlags::Color)
			.OnDirty(DF::Projection);
	}
	
	DECLARE_DIRTY_FLAGSET(Projection)
	
	glm::mat4 m_projectionMatrix;

	float m_aperture;

public:
	SpotLightNode(Node* parent);
	~SpotLightNode() = default;

	void UpdateProjectionMatrix();
	glm::mat4 GetProjectionMatrix() const { return m_projectionMatrix; }

	void DirtyUpdate() override;

	[[nodiscard]] float GetAperture() const { return m_aperture; }

	bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;
};
