#pragma once

#include "world/nodes/Node.h"

class LightNode : public Node {

	REFLECTED_NODE(LightNode, Node, DF_FLAGS(Flux, NearFar, ShadowsTextSize))
	{
		REFLECT_VAR(m_color, PropertyFlags::Color).OnDirty(DF::Flux);
		REFLECT_VAR(m_intensity).OnDirty(DF::Flux);

		REFLECT_VAR(m_hasShadow);

		REFLECT_VAR(m_near).OnDirty(DF::NearFar);
		REFLECT_VAR(m_far).OnDirty(DF::NearFar);

		REFLECT_VAR(m_shadowMapWidth).OnDirty(DF::ShadowsTextSize);
		REFLECT_VAR(m_shadowMapHeight).OnDirty(DF::ShadowsTextSize);

		REFLECT_VAR(m_maxShadowBias);
		REFLECT_VAR(m_samples);
	}

protected:
	glm::vec3 m_color{ glm::vec3(1.f) };
	float m_intensity{ 10.f };

	bool m_hasShadow{ true };

	int32 m_shadowMapWidth{ 2048 };
	int32 m_shadowMapHeight{ 2048 };

	float m_near{ 1.f };
	float m_far{ 15.5f };

	// TODO: consider moving stuff used by shaders as renderer reflected properties
	// e.g. maxShadowBias, samples etc
	float m_maxShadowBias{ 0.005f };

	int m_samples{ 4 };

public:
	[[nodiscard]] glm::vec3 GetColor() const { return m_color; }
	[[nodiscard]] float GetIntensity() const { return m_intensity; }
	[[nodiscard]] float GetMaxShadowBias() const { return m_maxShadowBias; }
	[[nodiscard]] int32 GetShadowMapWidth() const { return m_shadowMapWidth; }
	[[nodiscard]] int32 GetShadowMapHeight() const { return m_shadowMapHeight; }
	[[nodiscard]] float GetNear() const { return m_near; }
	[[nodiscard]] float GetFar() const { return m_far; }
	[[nodiscard]] bool CastsShadows() const { return m_hasShadow; }
	[[nodiscard]] int GetSamples() const { return m_samples; }
};
