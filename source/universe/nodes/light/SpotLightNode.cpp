#include "pch.h"
#include "SpotLightNode.h"

#include "rendering/scene/Scene.h"

void SpotLightNode::CalculateWorldAABB()
{
	m_aabb = m_frustum.FrustumPyramidAABB(GetNodePositionWCS());
}

void SpotLightNode::RecalculateProjectionMatrix()
{
	const auto ar = static_cast<float>(m_shadowMapWidth) / static_cast<float>(m_shadowMapHeight);
	m_projectionMatrix = glm::perspective(m_outerAperture, ar, m_near, m_far);
	// Vulkan's inverted y
	m_projectionMatrix[1][1] *= -1.f;
	RecalculateViewProjectionMatrix();
}

void SpotLightNode::RecalculateViewMatrix()
{
	const auto lookAt = GetNodePositionWCS() + GetNodeForwardWCS();
	// CHECK: Remove pragma when fixed in glm
#pragma warning(suppress : 4305)
	m_viewMatrix = glm::lookAt(GetNodePositionWCS(), lookAt, GetNodeUpWCS());

	RecalculateViewProjectionMatrix();
}

void SpotLightNode::RecalculateViewProjectionMatrix()
{
	m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;

	RecalculateFrustum();
}

void SpotLightNode::RecalculateFrustum()
{
	// viewProj to get frustum plane equations in world space
	m_frustum.ExtractFromMatrix(m_viewProjectionMatrix);
	CalculateWorldAABB();
}

SpotLightNode::SpotLightNode()
{
	sceneUid = Scene->EnqueueCreateCmd<SceneSpotlight>();
}

SpotLightNode::~SpotLightNode()
{
	Scene->EnqueueDestroyCmd<SceneSpotlight>(sceneUid);
}

void SpotLightNode::DirtyUpdate(DirtyFlagset flags)
{
	// constexpr glm::mat4 biasMatrix(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0);

	if (flags[DF::ShadowsTextSize]) {
		Enqueue([width = m_shadowMapWidth, height = m_shadowMapHeight](
					SceneSpotlight& sl) { sl.ResizeShadowmap(width, height); });
	}

	if (flags[DF::Aperture] || flags[DF::NearFar] || flags[DF::ShadowsTextSize]) {
		RecalculateProjectionMatrix();
		Enqueue([&](SceneSpotlight& sl) {
			sl.ubo.position = glm::vec4(GetNodePositionWCS(), 1.f);
			sl.ubo.forward = glm::vec4(GetNodeForwardWCS(), 1.f);
			sl.ubo.viewProj = m_viewProjectionMatrix;
			sl.ubo.color = glm::vec4(m_color, 1.f);
			sl.ubo.intensity = m_intensity;
			sl.ubo.near_ = m_near;
			sl.ubo.far_ = m_far;
			sl.ubo.outerCutOff = glm::cos(m_outerAperture / 2.f);
			sl.ubo.innerCutOff = glm::cos(m_innerAperture / 2.f);
			sl.ubo.constantTerm = m_contantTerm;
			sl.ubo.linearTerm = m_linearTerm;
			sl.ubo.quadraticTerm = m_quadraticTerm;
		});
	}

	if (flags[DF::SRT]) {
		RecalculateViewMatrix();
		Enqueue([&](SceneSpotlight& sl) {
			sl.ubo.position = glm::vec4(GetNodePositionWCS(), 1.f);
			sl.ubo.forward = glm::vec4(GetNodeForwardWCS(), 1.f);
			sl.ubo.viewProj = m_viewProjectionMatrix;
			sl.ubo.color = glm::vec4(m_color, 1.f);
			sl.ubo.intensity = m_intensity;
			sl.ubo.near_ = m_near;
			sl.ubo.far_ = m_far;
			sl.ubo.outerCutOff = glm::cos(m_outerAperture / 2.f);
			sl.ubo.innerCutOff = glm::cos(m_innerAperture / 2.f);
			sl.ubo.constantTerm = m_contantTerm;
			sl.ubo.linearTerm = m_linearTerm;
			sl.ubo.quadraticTerm = m_quadraticTerm;
		});
	}
}
