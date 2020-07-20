#include "pch.h"
#include "DirectionalLightNode.h"

#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneDirectionalLight.h"

void DirectionalLightNode::CalculateWorldAABB()
{
	m_aabb = m_frustum.FrustumPyramidAABB(GetNodePositionWCS());
}

void DirectionalLightNode::RecalculateProjectionMatrix()
{
	m_projectionMatrix = glm::ortho(m_left, m_right, m_bottom, m_top, m_near, m_far);
	// Vulkan's inverted y
	m_projectionMatrix[1][1] *= -1.f;
	RecalculateViewProjectionMatrix();
}

void DirectionalLightNode::RecalculateViewMatrix()
{
	const auto lookAt = GetNodePositionWCS() + GetNodeForwardWCS();
	m_viewMatrix = glm::lookAt(GetNodePositionWCS(), lookAt, GetNodeUpWCS());

	RecalculateViewProjectionMatrix();
}

void DirectionalLightNode::RecalculateViewProjectionMatrix()
{
	m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;

	RecalculateFrustum();
}

void DirectionalLightNode::RecalculateFrustum()
{
	// viewProj to get frustum plane equations in world space
	m_frustum.ExtractFromMatrix(m_viewProjectionMatrix);
	CalculateWorldAABB();
}

DirectionalLightNode::DirectionalLightNode()
{
	sceneUid = Scene->EnqueueCreateCmd<SceneDirectionalLight>();
}

DirectionalLightNode::~DirectionalLightNode()
{
	Scene->EnqueueDestroyCmd<SceneDirectionalLight>(sceneUid);
}

void DirectionalLightNode::DirtyUpdate(DirtyFlagset flags)
{
	if (flags[DF::Created]) {
		Enqueue([name = m_name](SceneDirectionalLight& dl) { dl.name = "depth: " + name; });
	}

	if (flags[DF::ShadowsTextSize]) {
		Enqueue([width = m_shadowMapWidth, height = m_shadowMapHeight](
					SceneDirectionalLight& dl) { dl.ResizeShadowmap(width, height); });
	}


	if (flags[DF::OrthoSides] || flags[DF::NearFar] || flags[DF::ShadowsTextSize]) {
		RecalculateProjectionMatrix();
		Enqueue([&](SceneDirectionalLight& dl) {
			dl.ubo.forward = glm::vec4(GetNodeForwardWCS(), 1.f);
			dl.ubo.viewProj = m_viewProjectionMatrix;
			dl.ubo.color = glm::vec4(m_color, 1.f);
			dl.ubo.intensity = m_intensity;
			dl.up = GetNodeUpWCS();
		});
	}

	if (flags[DF::SRT]) {
		RecalculateViewMatrix();
		Enqueue([&](SceneDirectionalLight& dl) {
			dl.ubo.forward = glm::vec4(GetNodeForwardWCS(), 1.f);
			dl.ubo.viewProj = m_viewProjectionMatrix;
			dl.ubo.color = glm::vec4(m_color, 1.f);
			dl.ubo.intensity = m_intensity;

			dl.up = GetNodeUpWCS();
			dl.ubo.maxShadowBias = m_maxShadowBias;
			dl.ubo.samples = m_samples;
			dl.ubo.sampleInvSpread = m_sampleInvSpread;
		});
	}
}
