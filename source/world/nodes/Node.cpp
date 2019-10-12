#include "pch/pch.h"

#include "world/nodes/Node.h"
#include "asset/util/ParsingAux.h"
#include "world/NodeFactory.h"
#include "asset/AssetManager.h"
#include "asset/PodIncludes.h"
#include "reflection/ReflectionTools.h"
#include "world/World.h"

RootNode* Node::GetWorldRoot() const
{
	return Engine::GetWorld()->GetRoot();
}

void Node::SetLocalTranslation(glm::vec3 lt)
{
	m_localTranslation = lt;
	MarkMatrixChanged();
}

void Node::SetLocalOrientation(glm::quat lo)
{
	m_localOrientation = lo;
	MarkMatrixChanged();
}

void Node::SetLocalScale(glm::vec3 ls)
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

	if (m_dirty[DF::TRS]) {

		LOG_REPORT("dirty {}", this->GetName());

		m_localMatrix = math::TransformMatrixFromTOS(m_localScale, m_localOrientation, m_localTranslation);
		m_worldMatrix = parentMatrix * m_localMatrix;
		// PERF:
		glm::vec3 skew;
		glm::vec4 persp;
		glm::decompose(m_worldMatrix, m_worldScale, m_worldOrientation, m_worldTranslation, skew, persp);

		m_obb = GetBBox();

		// m_obb.max = m_worldMatrix * glm::vec4(localBbox.max, 1.f);
		// m_obb.min = m_worldMatrix * glm::vec4(localBbox.min, 1.f);


		// TODO: rotated flag
		// apply scaling
		m_aabb.max = m_obb.max * m_worldScale;
		m_aabb.min = m_obb.min * m_worldScale;
		// m_aabb.max = localBbox.max * m_worldScale;
		// m_aabb.min = localBbox.min * m_worldScale;

		// apply translation
		m_aabb.max += m_obb.max + m_worldTranslation;
		m_aabb.min += m_obb.min + m_worldTranslation;
		// m_aabb.max = localBbox.max + m_worldTranslation;
		// m_aabb.min = localBbox.min + m_worldTranslation;
		//

		/*std::vector<glm::vec3> v;

		v.push_back(m_worldMatrix * glm::vec4{ m_obb.min.x, m_obb.min.y, m_obb.min.z, 1.f });
		v.push_back(m_worldMatrix * glm::vec4{ m_obb.max.x, m_obb.min.y, m_obb.min.z, 1.f });
		v.push_back(m_worldMatrix * glm::vec4{ m_obb.max.x, m_obb.max.y, m_obb.min.z, 1.f });
		v.push_back(m_worldMatrix * glm::vec4{ m_obb.min.x, m_obb.max.y, m_obb.min.z, 1.f });
		v.push_back(m_worldMatrix * glm::vec4{ m_obb.min.x, m_obb.min.y, m_obb.max.z, 1.f });
		v.push_back(m_worldMatrix * glm::vec4{ m_obb.max.x, m_obb.min.y, m_obb.max.z, 1.f });
		v.push_back(m_worldMatrix * glm::vec4{ m_obb.max.x, m_obb.max.y, m_obb.max.z, 1.f });
		v.push_back(m_worldMatrix * glm::vec4{ m_obb.min.x, m_obb.max.y, m_obb.max.z, 1.f });

		m_aabb = { v[0], v[0] };

		for (auto p : v) {
			m_aabb.max = glm::max(m_aabb.max, p);
			m_aabb.min = glm::min(m_aabb.min, p);
		}*/

		// calculate sphere

		// TODO: rotated flag
		// apply scaling
		// m_aabb.max = localBbox.max * m_worldScale;
		// m_aabb.min = localBbox.min * m_worldScale;

		// apply translation
		// m_aabb.max = localBbox.max + m_worldTranslation;
		// m_aabb.min = localBbox.min + m_worldTranslation;
		//
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

void Node::AddLocalOffset(glm::vec3 direction)
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
