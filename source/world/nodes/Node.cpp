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

void Node::SetLocalLookAt(glm::vec3 lookAt)
{
	SetLocalOrientation(math::OrientationFromLookatAndPosition(lookAt, m_localTranslation));
}

void Node::SetTranslation(glm::vec3 wt)
{
	auto parentMatrix = GetParent()->GetMatrix();
	SetLocalTranslation(glm::inverse(parentMatrix) * glm::vec4(wt, 1.f));
}

void Node::SetOrientation(glm::quat wo)
{
	auto worldMatrix = math::TransformMatrixFromTOS(m_scale, wo, m_translation);
	SetMatrix(worldMatrix);
}

void Node::SetEulerAngles(glm::vec3 pyr)
{
	SetOrientation(glm::quat(glm::radians(pyr)));
}

void Node::RotateAroundAxis(glm::vec3 worldAxis, float degrees)
{
	const glm::quat rot = glm::angleAxis(glm::radians(degrees), glm::vec3(worldAxis));
	SetOrientation(rot * m_orientation);
}

void Node::RotateAroundLocalAxis(glm::vec3 localAxis, float degrees)
{
	const glm::quat rot = glm::angleAxis(glm::radians(degrees), glm::vec3(localAxis));
	SetLocalOrientation(rot * m_localOrientation);
}

void Node::SetScale(glm::vec3 ws)
{
	auto worldMatrix = math::TransformMatrixFromTOS(ws, m_orientation, m_translation);
	SetMatrix(worldMatrix);
}

void Node::SetMatrix(const glm::mat4& newWorldMatrix)
{
	auto parentMatrix = GetParent()->GetMatrix();
	SetLocalMatrix(glm::inverse(parentMatrix) * newWorldMatrix);
}

void Node::SetLookAt(glm::vec3 lookAt)
{
	SetOrientation(math::OrientationFromLookatAndPosition(lookAt, m_translation));
}

void Node::CalculateWorldAABB()
{
	m_aabb = m_localBB;
	m_aabb.Transform(GetMatrix());
}

void Node::AutoUpdateTransforms()
{
	UpdateTransforms(GetParent()->GetMatrix());
}

void Node::UpdateTransforms(const glm::mat4& parentMatrix)
{
	m_dirty.set(DF::TRS);

	m_localMatrix = math::TransformMatrixFromTOS(m_localScale, m_localOrientation, m_localTranslation);
	m_matrix = parentMatrix * m_localMatrix;

	CalculateWorldAABB();

	// PERF:
	glm::vec3 skew;
	glm::vec4 persp;
	glm::decompose(m_matrix, m_scale, m_orientation, m_translation, skew, persp);

	for (auto& uPtr : m_children) {
		uPtr->UpdateTransforms(m_matrix);
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

void Node::AddOffset(glm::vec3 direction)
{
	SetTranslation(m_translation + direction);
}
