#pragma once

#include "world/nodes/light/LightNode.h"

class SpotLightNode : public LightNode {
	REFLECTED_NODE(SpotLightNode, LightNode, DF_FLAGS(Projection)) { REFLECT_VAR(m_aperture).OnDirty(DF::Projection); }

	glm::mat4 m_projectionMatrix{};

	float m_aperture{ 45.f };

	void UpdateProjectionMatrix();

public:
	void DirtyUpdate(DirtyFlagset flags) override;

	[[nodiscard]] glm::mat4 GetProjectionMatrix() const { return m_projectionMatrix; }
	[[nodiscard]] float GetAperture() const { return m_aperture; }
};
