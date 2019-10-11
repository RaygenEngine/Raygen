#include "pch/pch.h"

#include "world/nodes/Node.h"
#include "world/nodes/MetaNodeTranslation.h"
#include "asset/util/ParsingAux.h"
#include "user/freeform/FreeformUserNode.h"
#include "sky/SkyCubeNode.h"
#include "sky/SkyHDRNode.h"
#include "world/NodeFactory.h"
#include "asset/AssetManager.h"
#include "asset/PodIncludes.h"
#include "core/reflection/ReflectionTools.h"

RootNode* Node::GetWorldRoot() const
{
	return Engine::GetWorld()->GetRoot();
}

void Node::SetLocalTranslation(const glm::vec3& lt)
{
	m_localTranslation = lt;
	MarkMatrixChanged();
}

void Node::SetLocalOrientation(const glm::quat& lo)
{

	m_localOrientation = lo;
	MarkMatrixChanged();
}

void Node::SetLocalScale(const glm::vec3& ls)
{
	m_localScale = ls;
	MarkMatrixChanged();
}

void Node::SetLocalMatrix(const glm::mat4& lm)
{
	m_localMatrix = lm;

	glm::vec3 skew;
	glm::vec4 persp;
	glm::decompose(lm, m_localScale, m_localOrientation, m_localTranslation, skew, persp);
	MarkMatrixChanged();
}

void Node::SetWorldMatrix(const glm::mat4& newWorldMatrix)
{
	auto parentMatrix = GetParent()->GetWorldMatrix();

	SetLocalMatrix(glm::inverse(parentMatrix) * newWorldMatrix);
}

void Node::MarkMatrixChanged()
{
	m_dirty.set(DF::TRS);
	for (auto& child : m_children) {
		child->MarkMatrixChanged();
	}
}

void Node::UpdateTransforms(const glm::mat4& parentMatrix)
{
	if (m_dirty[DF::TRS]) {
		m_localMatrix = utl::GetTransformMat(m_localTranslation, m_localOrientation, m_localScale);
		m_worldMatrix = parentMatrix * m_localMatrix;
		// PERF:
		glm::vec3 skew;
		glm::vec4 persp;
		glm::decompose(m_worldMatrix, m_worldScale, m_worldOrientation, m_worldTranslation, skew, persp);
	}

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

void Node::AddLocalOffset(const glm::vec3& direction)
{
	m_localTranslation += direction;
	MarkMatrixChanged();
}

void Node::Orient(float yaw, float pitch, float roll)
{
	const glm::quat rotY = glm::angleAxis(yaw, glm::vec3(0.f, 1.f, 0.f));
	const glm::quat rotP = glm::angleAxis(pitch, glm::vec3(1.f, 0.f, 0.f));
	const glm::quat rotR = glm::angleAxis(roll, glm::vec3(0.f, 0.f, 1.f));

	m_localOrientation = rotY * rotP * rotR * m_localOrientation;

	MarkMatrixChanged();
}

void Node::OrientWithoutRoll(float yaw, float pitch)
{
	const glm::quat rotY = glm::angleAxis(yaw, glm::vec3(0.f, 1.f, 0.f));
	const glm::quat rotP = glm::angleAxis(pitch, glm::vec3(1.f, 0.f, 0.f));

	m_localOrientation = rotY * m_localOrientation * rotP;

	MarkMatrixChanged();
}

void Node::OrientYaw(float yaw)
{
	const glm::quat rotY = glm::angleAxis(yaw, glm::vec3(0.f, 1.f, 0.f));
	m_localOrientation = rotY * m_localOrientation;

	MarkMatrixChanged();
}
