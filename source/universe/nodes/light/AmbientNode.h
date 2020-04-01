#pragma once
#include "assets/pods/Sampler.h"
#include "universe/nodes/Node.h"

class AmbientNode : public Node {
	REFLECTED_NODE(AmbientNode, Node, DF_FLAGS(SkyTexture))
	{
		REFLECT_VAR(m_ambientTerm, PropertyFlags::Color);
		REFLECT_VAR(m_skybox).OnDirty(DF::SkyTexture);
	}

	PodHandle<Sampler> m_skybox;

	glm::vec3 m_ambientTerm{ 0.02f, 0.02f, 0.02f };

public:
	[[nodiscard]] PodHandle<Sampler> GetSkybox() const { return m_skybox; }
	[[nodiscard]] glm::vec3 GetAmbientTerm() const { return m_ambientTerm; }
};
