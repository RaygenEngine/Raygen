#pragma once

#include "world/nodes/light/LightNode.h"

class SpotLightNode : public LightNode {
	REFLECTED_NODE(SpotLightNode, LightNode, DF_FLAGS(Projection))
	{
		REFLECT_VAR(m_outerAperture).OnDirty(DF::Projection);
		REFLECT_VAR(m_innerAperture);
	}

	glm::mat4 m_projectionMatrix{};
	glm::mat4 m_viewMatrix{};
	glm::mat4 m_viewProjectionMatrix{};

	// angle
	float m_outerAperture{ 25.f };
	// inner
	float m_innerAperture{ 12.5f };

	void RecalculateProjectionMatrix();
	void RecalculateViewMatrix();
	void RecalculateViewProjectionMatrix();

public:
	void DirtyUpdate(DirtyFlagset flags) override;

	// WIP: return radians or keep radians in general
	[[nodiscard]] float GetOuterAperture() const { return m_outerAperture; }
	[[nodiscard]] float GetInnerAperture() const { return m_innerAperture; }

	[[nodiscard]] glm::mat4 GetProjectionMatrix() const { return m_projectionMatrix; }
	[[nodiscard]] glm::mat4 GetViewMatrix() const { return m_viewMatrix; }
	[[nodiscard]] glm::mat4 GetViewProjectionMatrix() const { return m_viewProjectionMatrix; }
};
