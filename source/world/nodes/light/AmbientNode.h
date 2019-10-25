#pragma once

#include "world/nodes/Node.h"
#include "asset/pods/TexturePod.h"

class AmbientNode : public Node {
	REFLECTED_NODE(AmbientNode, Node, DF_FLAGS(SkyTexture))
	{
		REFLECT_VAR(m_ambientTerm, PropertyFlags::Color);
		REFLECT_VAR(m_skybox).OnDirty(DF::SkyTexture);
	}

	PodHandle<TexturePod> m_skybox;

	glm::vec3 m_ambientTerm{ 0.7f, 0.7f, 0.7f };

public:
	[[nodiscard]] PodHandle<TexturePod> GetSkybox() const { return m_skybox; }
	[[nodiscard]] glm::vec3 GetAmbientTerm() const { return m_ambientTerm; }
};
