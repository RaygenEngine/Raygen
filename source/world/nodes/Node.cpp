#include "pch/pch.h"

#include "world/nodes/Node.h"
#include "asset/util/ParsingAux.h"
#include "asset/AssetManager.h"
#include "reflection/ReflectionTools.h"
#include "world/World.h"
#include "core/MathAux.h"

#include <glm/gtx/matrix_decompose.hpp>

RootNode* Node::GetWorldRoot() const
{
	return Engine::GetWorld()->GetRoot();
}

void Node::SetNodePositionLCS(glm::vec3 lt)
{
	m_localPosition = lt;
	AutoUpdateTransforms();
}

void Node::SetNodeOrientationLCS(glm::quat lo)
{
	m_localOrientation = lo;
	AutoUpdateTransforms();
}

void Node::SetNodeEulerAnglesLCS(glm::vec3 pyr)
{
	SetNodeOrientationLCS(glm::quat(glm::radians(pyr)));
}

void Node::SetNodeScaleLCS(glm::vec3 ls)
{
	m_localScale = ls;
	AutoUpdateTransforms();
}

void Node::SetNodeTransformLCS(const glm::mat4& lm)
{
	m_localTransform = lm;

	glm::vec3 skew;
	glm::vec4 persp;
	glm::decompose(lm, m_localScale, m_localOrientation, m_localPosition, skew, persp);
	AutoUpdateTransforms();
}

void Node::SetNodeLookAtLCS(glm::vec3 lookAt)
{
	SetNodeOrientationLCS(math::OrientationFromLookatAndPosition(lookAt, m_localPosition));
}

void Node::SetNodePositionWCS(glm::vec3 wt)
{
	auto parentMatrix = GetParent()->GetNodeTransformWCS();
	SetNodePositionLCS(glm::inverse(parentMatrix) * glm::vec4(wt, 1.f));
}

void Node::SetNodeOrientationWCS(glm::quat wo)
{
	auto worldMatrix = math::TransformMatrixFromSOT(m_scale, wo, m_position);
	SetNodeTransformWCS(worldMatrix);
}

void Node::SetNodeEulerAnglesWCS(glm::vec3 pyr)
{
	SetNodeOrientationWCS(glm::quat(glm::radians(pyr)));
}

void Node::RotateNodeAroundAxisWCS(glm::vec3 worldAxis, float degrees)
{
	const glm::quat rot = glm::angleAxis(glm::radians(degrees), glm::vec3(worldAxis));
	SetNodeOrientationWCS(rot * m_orientation);
}

void Node::RotateNodeAroundAxisLCS(glm::vec3 localAxis, float degrees)
{
	const glm::quat rot = glm::angleAxis(glm::radians(degrees), glm::vec3(localAxis));
	SetNodeOrientationLCS(rot * m_localOrientation);
}

void Node::SetNodeScaleWCS(glm::vec3 ws)
{
	auto worldMatrix = math::TransformMatrixFromSOT(ws, m_orientation, m_position);
	SetNodeTransformWCS(worldMatrix);
}

void Node::SetNodeTransformWCS(const glm::mat4& newWorldMatrix)
{
	auto parentMatrix = GetParent()->GetNodeTransformWCS();
	SetNodeTransformLCS(glm::inverse(parentMatrix) * newWorldMatrix);
}

void Node::SetNodeLookAtWCS(glm::vec3 lookAt)
{
	SetNodeOrientationWCS(math::OrientationFromLookatAndPosition(lookAt, m_position));
}

void Node::CalculateWorldAABB()
{
	m_aabb = m_localBB;
	m_aabb.Transform(GetNodeTransformWCS());
}

void Node::AutoUpdateTransforms()
{
	UpdateTransforms(GetParent()->GetNodeTransformWCS());
}

void Node::UpdateTransforms(const glm::mat4& parentMatrix)
{
	m_dirty.set(DF::SRT);

	m_localTransform = math::TransformMatrixFromSOT(m_localScale, m_localOrientation, m_localPosition);
	m_transform = parentMatrix * m_localTransform;

	CalculateWorldAABB();

	// PERF:
	glm::vec3 skew;
	glm::vec4 persp;
	glm::decompose(m_transform, m_scale, m_orientation, m_position, skew, persp);

	for (auto& uPtr : m_children) {
		uPtr->UpdateTransforms(m_transform);
	}
}

void Node::DeleteChild(Node* child)
{
	auto it = std::find_if(m_children.begin(), m_children.end(), [&](auto& ref) { return ref.get() == child; });
	m_children.erase(it);
	m_dirty.set(DF::Children);
}

void Node::AddNodePositionOffsetLCS(glm::vec3 offset)
{
	SetNodePositionLCS(m_localPosition + offset);
}

void Node::AddNodePositionOffsetWCS(glm::vec3 offset)
{
	SetNodePositionWCS(m_position + offset);
}
