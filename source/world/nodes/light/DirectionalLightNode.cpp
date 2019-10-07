#include "pch.h"

#include "world/nodes/light/DirectionalLightNode.h"
#include "asset/util/ParsingAux.h"


DirectionalLightNode::DirectionalLightNode(Node* parent)
	: LightNode(parent),
      m_orthoProjectionMatrix()
{
}

bool DirectionalLightNode::LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData)
{
	LightNode::LoadAttributesFromXML(xmlData);

	glm::vec3 localLookat{};
	if (ParsingAux::ReadFloatsAttribute(xmlData, "lookat", localLookat))
	{
		// if lookat read overwrite following
		SetLocalOrientation(utl::GetOrientationFromLookAtAndPosition(localLookat, GetLocalTranslation()));
	}
	glm::ortho(m_left, m_right, m_bottom, m_top, m_near, m_far);
	return true;
}

void DirectionalLightNode::DirtyUpdate()
{
	LightNode::DirtyUpdate();
	
	if (m_dirty[DF::Projection])
	{
		UpdateOrthoProjectionMatrix();
	}
}

