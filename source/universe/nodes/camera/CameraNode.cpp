#include "pch.h"
#include "CameraNode.h"


void CameraNode::EnqueueActiveCamera()
{
	Scene->EnqueueActiveCameraCmd(sceneUid);
}

CameraNode::CameraNode()
{
	sceneUid = Scene->EnqueueCreateCmd<SceneCamera>();
}

CameraNode::~CameraNode()
{
	Scene->EnqueueDestroyCmd<SceneCamera>(sceneUid);
}

void CameraNode::CalculateWorldAABB()
{
	m_aabb = m_frustum.FrustumPyramidAABB(GetNodePositionWCS());
}

void CameraNode::RecalculateProjectionFov()
{
	const auto ar = static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight);
	m_hFov = 2 * atan(ar * tan(m_vFov * 0.5f));

	const auto top = tan(m_vFov / 2.f + m_vFovOffset) * m_near;
	const auto bottom = tan(-m_vFov / 2.f - m_vFovOffset) * m_near;

	const auto right = tan(m_hFov / 2.f + m_hFovOffset) * m_near;
	const auto left = tan(-m_hFov / 2.f - m_hFovOffset) * m_near;

	m_projectionMatrix = glm::frustum(left, right, bottom, top, m_near, m_far);
	// Vulkan's inverted y
	m_projectionMatrix[1][1] *= -1.f;
	RecalculateViewProjectionMatrix();
}

void CameraNode::RecalculateViewMatrix()
{
	m_viewMatrix = glm::lookAt(GetNodePositionWCS(), GetLookAt(), GetNodeUpWCS());
	RecalculateViewProjectionMatrix();
}

void CameraNode::RecalculateViewProjectionMatrix()
{
	m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;

	// viewProj to get frustum plane equations in world space
	m_frustum.ExtractFromMatrix(m_viewProjectionMatrix);
	CalculateWorldAABB();
}

void CameraNode::DirtyUpdate(DirtyFlagset flags)
{
	Node::DirtyUpdate(flags);
	if (flags[DF::Projection] || flags[DF::ViewportSize]) {
		RecalculateProjectionFov();

		Enqueue([&](SceneCamera& cam) {
			cam.ubo.position = glm::vec4(GetNodePositionWCS(), 1.f);
			cam.ubo.view = m_viewMatrix;
			cam.ubo.proj = m_projectionMatrix;
			cam.ubo.viewProj = m_viewProjectionMatrix;
			cam.ubo.viewInv = glm::inverse(m_viewMatrix);
			cam.ubo.projInv = glm::inverse(m_projectionMatrix);
			cam.ubo.viewProjInv = glm::inverse(m_viewProjectionMatrix);
		});
	}

	if (flags[DF::SRT] || flags[DF::FocalLength]) {
		RecalculateViewMatrix();

		Enqueue([&](SceneCamera& cam) {
			cam.ubo.position = glm::vec4(GetNodePositionWCS(), 1.f);
			cam.ubo.view = m_viewMatrix;
			cam.ubo.proj = m_projectionMatrix;
			cam.ubo.viewProj = m_viewProjectionMatrix;
			cam.ubo.viewInv = glm::inverse(m_viewMatrix);
			cam.ubo.projInv = glm::inverse(m_projectionMatrix);
			cam.ubo.viewProjInv = glm::inverse(m_viewProjectionMatrix);
		});
	}
}
