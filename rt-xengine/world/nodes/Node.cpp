#include "pch.h"

#include "world/nodes/Node.h"
#include "world/nodes/MetaNodeTranslation.h"
#include "assets/other/xml/ParsingAux.h"
#include "user/freeform/FreeformUserNode.h"
#include "sky/SkyCubeNode.h"
#include "sky/SkyHDRNode.h"
#include "world/NodeFactory.h"

AssetManager* Node::GetAssetManager() const
{
	return GetWorld()->GetAssetManager();
}

Node::Node(Node* pNode)
	: Object(),
		m_localTranslation(0.f, 0.f, 0.f),
		m_localOrientation(1.f, 0.f, 0.f, 0.f),
		// note w, x, y, z
		m_localScale(1.f, 1.f, 1.f),
		m_localMatrix(glm::mat4(1.f)),
		m_worldTranslation(0.f, 0.f, 0.f),
		m_worldOrientation(1.f, 0.f, 0.f, 0.f),
		m_worldScale(1.f, 1.f, 1.f),
		m_worldMatrix(glm::mat4(1.f)),
		m_dirty(false),
		m_updateLocalMatrix(true),
	    // update local on first iter
	    m_parent(pNode)
{
	if (pNode) {
		m_world = pNode->GetWorld();
	}
}

void Node::SetLocalTranslation(const glm::vec3& lt)
{
	m_localTranslation = lt;

	m_updateLocalMatrix = true;
	MarkDirty();
}

void Node::SetLocalOrientation(const glm::quat& lo)
{
	m_localOrientation = lo;

	m_updateLocalMatrix = true;
	MarkDirty();
}

void Node::SetLocalScale(const glm::vec3& ls)
{
	m_localScale = ls;

	m_updateLocalMatrix = true;
	MarkDirty();
}

void Node::SetLocalMatrix(const glm::mat4& lm)
{
	m_localMatrix = lm;

	MarkDirty();
}

void Node::CacheWorldTransform()
{
	// root
	if (!m_parent)
		return;
		
	if (!m_dirty)
		return;

	// cache from here until root
	m_parent->CacheWorldTransform();

	if(m_updateLocalMatrix)
		m_localMatrix = Core::GetTransformMat(m_localTranslation, m_localOrientation, m_localScale);

	m_worldMatrix = m_parent->GetWorldMatrix() * m_localMatrix;

	// TODO optimize
	glm::vec3 skew; glm::vec4 persp;
	glm::decompose(m_worldMatrix, m_worldScale, m_worldOrientation, m_worldTranslation, skew, persp);

	m_dirty = false;
	m_updateLocalMatrix = false;

}

void Node::MarkDirty()
{
	if(m_dirty)
		return;

	m_dirty = true;

	GetWorld()->AddDirtyNode(this);
	for (auto& child : m_children)
		child->MarkDirty();
}

bool Node::LoadFromXML(const tinyxml2::XMLElement* xmlData)
{
	ParsingAux::ReadFillEntityName(xmlData, m_name);
	ParsingAux::ReadFillEntityType(xmlData, m_type);

	LOG_INFO("Loading {0} named {1}", m_type, m_name);

	NodeFactory* factory = GetWorld()->GetNodeFactory();

	const auto status = LoadAttributesFromXML(xmlData) && factory->LoadChildren(xmlData, this);
		
	// calculate local matrix after loading
	m_localMatrix = Core::GetTransformMat(m_localTranslation, m_localOrientation, m_localScale);

	return status;
}

bool Node::LoadAttributesFromXML(const tinyxml2::XMLElement * xmlData)
{
	ParsingAux::ReadFloatsAttribute(xmlData, "translation", m_localTranslation);
	glm::vec3 eulerPYR{ 0.f, 0.f, 0.f };
	ParsingAux::ReadFloatsAttribute(xmlData, "euler_pyr", eulerPYR);
	m_localOrientation = glm::quat(glm::radians(eulerPYR));
	ParsingAux::ReadFloatsAttribute(xmlData, "scale", m_localScale);

	m_updateLocalMatrix = true;
	MarkDirty();

	return true;
}

void Node::Move(const glm::vec3& direction, float magnitude)
{
	m_localTranslation += direction * magnitude * GetWorld()->GetDeltaTime();

	m_updateLocalMatrix = true;
	MarkDirty();
}

void Node::MoveUp(float magnitude)
{
	m_localTranslation += GetUp() * magnitude * GetWorld()->GetDeltaTime();

	m_updateLocalMatrix = true;
	MarkDirty();
}

void Node::MoveDown(float magnitude)
{
	m_localTranslation += -GetUp() * magnitude * GetWorld()->GetDeltaTime();

	m_updateLocalMatrix = true;
	MarkDirty();
}

void Node::MoveRight(float magnitude)
{
	m_localTranslation += GetRight() * magnitude * GetWorld()->GetDeltaTime();

	m_updateLocalMatrix = true;
	MarkDirty();
}

void Node::MoveLeft(float magnitude)
{
	m_localTranslation += -GetRight() * magnitude * GetWorld()->GetDeltaTime();

	m_updateLocalMatrix = true;
	MarkDirty();
}

void Node::MoveFront(float magnitude)
{
	m_localTranslation += GetFront() * magnitude * GetWorld()->GetDeltaTime();

	m_updateLocalMatrix = true;
	MarkDirty();
}

void Node::MoveBack(float magnitude)
{
	m_localTranslation += -GetFront() * magnitude * GetWorld()->GetDeltaTime();

	m_updateLocalMatrix = true;
	MarkDirty();
}

// TODO: orient speed adjust by world delta?
void Node::Orient(float yaw, float pitch, float roll)
{
	const glm::quat rotY = glm::angleAxis(yaw, glm::vec3(0.f, 1.f, 0.f));
	const glm::quat rotP = glm::angleAxis(pitch, glm::vec3(1.f, 0.f, 0.f));
	const glm::quat rotR = glm::angleAxis(roll, glm::vec3(0.f, 0.f, 1.f));

	m_localOrientation = rotY * rotP * rotR * m_localOrientation;

	m_updateLocalMatrix = true;
	MarkDirty();
}

void Node::OrientWithoutRoll(float yaw, float pitch)
{
	const glm::quat rotY = glm::angleAxis(yaw, glm::vec3(0.f, 1.f, 0.f));
	const glm::quat rotP = glm::angleAxis(pitch, glm::vec3(1.f, 0.f, 0.f));

	m_localOrientation = rotY * m_localOrientation * rotP;

	m_updateLocalMatrix = true;
	MarkDirty();
}

void Node::OrientYaw(float yaw)
{
	const glm::quat rotY = glm::angleAxis(yaw, glm::vec3(0.f, 1.f, 0.f));

	m_localOrientation = rotY * m_localOrientation;

	m_updateLocalMatrix = true;
	MarkDirty();
}

void Node::Update(float deltaTime)
{

}

std::string Node::ToString(bool verbose, uint depth) const
{
	std::string moreInfo{};
	if (verbose)
	{
		const void* address = static_cast<const void*>(this);
		std::stringstream ss;
		ss << address;
		moreInfo = " uuid:" + std::to_string(GetUID()) + " address:" + ss.str();
	}
	std::string groupTree{};
	for (const auto child : m_children)
		groupTree += child->ToString(verbose, depth + 1);
	return "name:" + m_name + moreInfo + "\n" + groupTree;
}
