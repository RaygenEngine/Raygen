#pragma once

#include "world/nodes/light/LightNode.h"
#include "core/MathAux.h"


class SpotLightNode : public LightNode {
	REFLECTED_NODE(SpotLightNode, LightNode, DF_FLAGS(Aperture))
	{
		REFLECT_VAR(m_outerAperture, PropertyFlags::Rads).OnDirty(DF::Aperture);
		REFLECT_VAR(m_innerAperture, PropertyFlags::Rads);

		REFLECT_VAR(m_attenuationMode);
	}

	glm::mat4 m_projectionMatrix{};
	glm::mat4 m_viewMatrix{};
	glm::mat4 m_viewProjectionMatrix{};
	// world space frustum TODO
	math::Frustum m_frustum{};

	// angle
	float m_outerAperture{ glm::radians(45.f) };
	// inner
	float m_innerAperture{ glm::radians(22.5f) };

	enum AttenuationMode : int32
	{
		CONSTANT = 0,
		LINEAR = 1,
		QUADRATIC = 2
	} m_attenuationMode{ LINEAR };

	void CalculateWorldAABB() override;

	void RecalculateProjectionMatrix();
	void RecalculateViewMatrix();
	void RecalculateViewProjectionMatrix();
	void RecalculateFrustum();

public:
	void DirtyUpdate(DirtyFlagset flags) override;

	[[nodiscard]] float GetOuterAperture() const { return m_outerAperture; }
	[[nodiscard]] float GetInnerAperture() const { return m_innerAperture; }

	[[nodiscard]] glm::mat4 GetProjectionMatrix() const { return m_projectionMatrix; }
	[[nodiscard]] glm::mat4 GetViewMatrix() const { return m_viewMatrix; }
	[[nodiscard]] glm::mat4 GetViewProjectionMatrix() const { return m_viewProjectionMatrix; }
	[[nodiscard]] AttenuationMode GetAttenuationMode() const { return m_attenuationMode; }
	//[[nodiscard]] math::Frustum GetFrustum() const { return m_frustum; }

	bool IsNodeInsideFrustum(Node* node) { return m_frustum.IntersectsAABB(node->GetAABB()); }
};
