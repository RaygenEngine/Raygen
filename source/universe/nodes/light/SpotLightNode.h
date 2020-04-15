#pragma once
#include "core/math-ext/Frustum.h"
#include "universe/nodes/light/LightNode.h"


class SpotLightNode : public LightNode {
	REFLECTED_NODE(SpotLightNode, LightNode, DF_FLAGS(Aperture))
	{
		REFLECT_VAR(m_outerAperture, PropertyFlags::Rads).OnDirty(DF::Aperture);
		REFLECT_VAR(m_innerAperture, PropertyFlags::Rads);

		REFLECT_VAR(m_contantTerm);
		REFLECT_VAR(m_linearTerm);
		REFLECT_VAR(m_quadraticTerm);
	}

	glm::mat4 m_projectionMatrix{};
	glm::mat4 m_viewMatrix{};
	glm::mat4 m_viewProjectionMatrix{};
	math::Frustum m_frustum{};

	float m_contantTerm{ 1.f };
	float m_linearTerm{ 1.f };
	float m_quadraticTerm{ 1.f };

	// angle
	float m_outerAperture{ glm::radians(45.f) };
	// inner
	float m_innerAperture{ glm::radians(22.5f) };

	void CalculateWorldAABB() override;

	void RecalculateProjectionMatrix();
	void RecalculateViewMatrix();
	void RecalculateViewProjectionMatrix();
	void RecalculateFrustum();

public:
	SpotLightNode();
	~SpotLightNode() override;

	void DirtyUpdate(DirtyFlagset flags) override;

	[[nodiscard]] float GetOuterAperture() const { return m_outerAperture; }
	[[nodiscard]] float GetInnerAperture() const { return m_innerAperture; }

	[[nodiscard]] glm::mat4 GetProjectionMatrix() const { return m_projectionMatrix; }
	[[nodiscard]] glm::mat4 GetViewMatrix() const { return m_viewMatrix; }
	[[nodiscard]] glm::mat4 GetViewProjectionMatrix() const { return m_viewProjectionMatrix; }
	//[[nodiscard]] math::Frustum GetFrustum() const { return m_frustum; }

	bool IsNodeInsideFrustum(Node* node) { return m_frustum.Intersects(node->GetAABB()); }

private:
	size_t sceneUid;
	template<typename Lambda>
	void Enqueue(Lambda&& l)
	{
		Scene->EnqueueCmd<typename SceneSpotlight>(sceneUid, l);
	}
};
