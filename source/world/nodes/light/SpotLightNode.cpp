#include "pch.h"

#include "world/nodes/light/SpotLightNode.h"
#include "asset/util/ParsingAux.h"


SpotLightNode::SpotLightNode(Node* parent)
	: LightNode(parent),
	  m_projectionMatrix(),
      m_aperture(45.f)
{
}

void SpotLightNode::UpdateProjectionMatrix()
{
	auto ar = static_cast<float>(m_shadowMapWidth) / static_cast<float>(m_shadowMapHeight);
	m_projectionMatrix = glm::perspective(glm::radians(m_aperture), ar, m_near, m_far);
}


bool SpotLightNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
{
	LightNode::LoadAttributesFromXML(xmlData);
	// WIP:


	return true;
}

void SpotLightNode::DirtyUpdate()
{
	LightNode::DirtyUpdate();

	if (m_dirty[DF::Projection])
	{
		UpdateProjectionMatrix();
	}
}



