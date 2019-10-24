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

void Node::SetLocalTranslation(glm::vec3 lt)
{
	m_localTranslation = lt;
	AutoUpdateTransforms();
}

void Node::SetLocalOrientation(glm::quat lo)
{
	m_localOrientation = lo;
	AutoUpdateTransforms();
}

void Node::SetLocalPYR(glm::vec3 pyr)
{
	SetLocalOrientation(glm::quat(glm::radians(pyr)));
}

void Node::SetLocalScale(glm::vec3 ls)
{
	m_localScale = ls;
	AutoUpdateTransforms();
}

void Node::SetLocalMatrix(const glm::mat4& lm)
{
	m_localMatrix = lm;

	glm::vec3 skew;
	glm::vec4 persp;
	glm::decompose(lm, m_localScale, m_localOrientation, m_localTranslation, skew, persp);
	AutoUpdateTransforms();
}

void Node::SetWorldTranslation(glm::vec3 wt)
{
	auto parentMatrix = GetParent()->GetWorldMatrix();
	SetLocalTranslation(glm::inverse(parentMatrix) * glm::vec4(wt, 1.f));
}

void Node::SetWorldOrientation(glm::quat wo)
{
	auto worldMatrix = math::TransformMatrixFromTOS(m_worldScale, wo, m_worldTranslation);
	SetWorldMatrix(worldMatrix);
}

void Node::SetWorldPYR(glm::vec3 pyr)
{
	SetWorldOrientation(glm::quat(glm::radians(pyr)));
}

void Node::RotateAroundAxis(glm::vec3 worldAxis, float degrees)
{
	const glm::quat rot = glm::angleAxis(glm::radians(degrees), glm::vec3(worldAxis));
	SetWorldOrientation(rot * m_worldOrientation);
}

void Node::SetWorldScale(glm::vec3 ws)
{
	auto worldMatrix = math::TransformMatrixFromTOS(ws, m_worldOrientation, m_worldTranslation);
	SetWorldMatrix(worldMatrix);
}

void Node::SetWorldMatrix(const glm::mat4& newWorldMatrix)
{
	auto parentMatrix = GetParent()->GetWorldMatrix();
	SetLocalMatrix(glm::inverse(parentMatrix) * newWorldMatrix);
}

void Node::SetLookAt(glm::vec3 lookat)
{
	SetWorldOrientation(math::OrientationFromLookatAndPosition(lookat, m_worldTranslation));
}

void Node::CalculateWorldAABB()
{
	m_aabb = m_localBB;
	m_aabb.Transform(GetWorldMatrix());
}

void Node::AutoUpdateTransforms()
{
	UpdateTransforms(GetParent()->GetWorldMatrix());
}

void Node::UpdateTransforms(const glm::mat4& parentMatrix)
{
	m_dirty.set(DF::TRS);

	m_localMatrix = math::TransformMatrixFromTOS(m_localScale, m_localOrientation, m_localTranslation);
	m_worldMatrix = parentMatrix * m_localMatrix;

	CalculateWorldAABB();

	// PERF:
	glm::vec3 skew;
	glm::vec4 persp;
	glm::decompose(m_worldMatrix, m_worldScale, m_worldOrientation, m_worldTranslation, skew, persp);

	for (auto& uPtr : m_children) {
		uPtr->UpdateTransforms(m_worldMatrix);
	}
}

void Node::DeleteChild(Node* child)
{
	auto it = std::find_if(m_children.begin(), m_children.end(), [&](auto& ref) { return ref.get() == child; });
	m_children.erase(it);
	m_dirty.set(DF::Children);
}

void Node::AddLocalOffset(glm::vec3 direction)
{
	SetLocalTranslation(m_localTranslation + direction);
}

void Node::AddWorldOffset(glm::vec3 direction)
{
	SetWorldTranslation(m_worldTranslation + direction);
}
