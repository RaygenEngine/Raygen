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
	auto parentTranslation = GetParent()->GetWorldTranslation();
	SetLocalTranslation(wt - parentTranslation);
}

void Node::SetWorldOrientation(glm::quat wo)
{
	auto parentOrient = GetParent()->GetWorldOrientation();
	SetLocalOrientation(glm::inverse(parentOrient) * wo);
}

void Node::RotateAroundAxis(glm::vec3 worldAxis, float degrees)
{
	const glm::quat rot = glm::angleAxis(glm::radians(degrees), glm::vec3(worldAxis));
	SetWorldOrientation(rot * m_worldOrientation);
}

void Node::SetWorldScale(glm::vec3 ws)
{
	auto parentScale = GetParent()->GetWorldScale();
	SetLocalScale(ws / parentScale);
}

void Node::SetWorldMatrix(const glm::mat4& newWorldMatrix)
{
	auto parentMatrix = GetParent()->GetWorldMatrix();
	SetLocalMatrix(glm::inverse(parentMatrix) * newWorldMatrix);
}

void Node::AutoUpdateTransforms()
{
	UpdateTransforms(GetParent()->GetWorldMatrix());
}

void Transform(Box& box, const glm::mat4& M)
{
	float a, b;

	// Copy box A into min and max array.
	auto AMin = box.min;
	auto AMax = box.max;

	// Begin at T.
	box.max = glm::vec3(M[3]);
	box.min = glm::vec3(M[3]);

	// Find extreme points by considering product of
	// min and max with each component of M.
	for (auto j = 0; j < 3; ++j) {
		for (auto i = 0; i < 3; ++i) {
			auto a = M[i][j] * AMin[j];
			auto b = M[i][j] * AMax[j];

			if (a < b) {
				box.min[j] += a;
				box.max[j] += b;
			}
			else {
				box.min[j] += b;
				box.max[j] += a;
			}
		}
	}
}

void Node::UpdateTransforms(const glm::mat4& parentMatrix)
{
	m_dirty.set(DF::TRS);

	m_localMatrix = math::TransformMatrixFromTOS(m_localScale, m_localOrientation, m_localTranslation);
	m_worldMatrix = parentMatrix * m_localMatrix;
	// PERF:
	glm::vec3 skew;
	glm::vec4 persp;
	glm::decompose(m_worldMatrix, m_worldScale, m_worldOrientation, m_worldTranslation, skew, persp);

	auto localBbox = GetBBox();

	m_obb.max = m_worldMatrix * glm::vec4(localBbox.max, 1.f);
	m_obb.min = m_worldMatrix * glm::vec4(localBbox.min, 1.f);

	// calculate sphere

	// TODO: rotated flag
	// apply scaling
	m_aabb.max = localBbox.max * m_worldScale;
	m_aabb.min = localBbox.min * m_worldScale;

	// apply translation
	m_aabb.max = localBbox.max + m_worldTranslation;
	m_aabb.min = localBbox.min + m_worldTranslation;

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
