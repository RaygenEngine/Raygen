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

bool Node::LoadFromXML(const tinyxml2::XMLElement* xmlData)
{
	parsingaux::ReadFillEntityName(xmlData, m_name);
	parsingaux::ReadFillEntityType(xmlData, m_type);

	LOG_INFO("Loading {0} named {1}", m_type, m_name);


	// Load from reflection
	LoadReflectedProperties(xmlData);

	// Load Node generics
	parsingaux::ReadFloatsAttribute(xmlData, "translation", m_localTranslation);
	glm::vec3 eulerPYR{ 0.f, 0.f, 0.f };
	parsingaux::ReadFloatsAttribute(xmlData, "euler_pyr", eulerPYR);
	m_localOrientation = glm::quat(glm::radians(eulerPYR));
	parsingaux::ReadFloatsAttribute(xmlData, "scale", m_localScale);

	glm::vec3 localLookat{};
	if (parsingaux::ReadFloatsAttribute(xmlData, "lookat", localLookat)) {
		// if lookat read overwrite following
		m_localOrientation = math::OrientationFromLookatAndPosition(localLookat, GetLocalTranslation());
	}

	// Load Children.
	NodeFactory* factory = Engine::GetWorld()->GetNodeFactory();
	const auto status = factory->LoadChildrenXML(xmlData, this);

	m_dirty.set();

	return status;
}

namespace {

struct LoadPropertiesFromXMLVisitor {
	const tinyxml2::XMLElement* xmlData;
	const char* str;
	std::string strBuf;
	PodHandle<XMLDocPod> worldHandle;

	bool PreProperty(const Property& p)
	{
		if (p.HasFlags(PropertyFlags::NoLoad)) {
			return false;
		}
		strBuf = p.GetNameStr();
		str = strBuf.c_str();
		return true;
	}

	// Basic types
	void operator()(int32& v, const Property& p) { xmlData->QueryIntAttribute(str, &v); }
	void operator()(bool& v, const Property& p) { xmlData->QueryBoolAttribute(str, &v); }
	void operator()(float& v, const Property& p) { xmlData->QueryFloatAttribute(str, &v); }

	void operator()(glm::vec3& v, const Property& p) { parsingaux::ReadFloatsAttribute(xmlData, str, v); }
	void operator()(glm::vec4& v, const Property& p) { parsingaux::ReadFloatsAttribute(xmlData, str, v); }

	void operator()(std::string& v, const Property& p) { parsingaux::ReadStringAttribute(xmlData, str, v); }

	// Pod<T>
	template<typename T>
	void operator()(PodHandle<T>& handle, const Property& p)
	{
		std::string tmp;
		if (!parsingaux::ReadStringAttribute(xmlData, str, tmp)) {
			LOG_ABORT("Failed to load non optional pod: {} (Attribute missing in scene file).", p.GetName());
		}
		handle = AssetManager::GetOrCreateFromParent<T>(tmp, worldHandle);
	}


	void operator()(PodHandle<ModelPod>& handle, const Property& p)
	{
		std::string tmp;
		if (!parsingaux::ReadStringAttribute(xmlData, str, tmp)) {
			LOG_ABORT("Failed to load non optional pod: {} (Attribute missing in scene file).", p.GetName());
		}
		handle = AssetManager::GetOrCreateFromParent<ModelPod>(tmp, worldHandle);

		if (tmp.find(".gltf") != std::string::npos) {
			AssetManager::PreloadGltf(AssetManager::GetPodUri(handle) + "{}");
		}
	}

	template<typename T>
	void operator()(std::vector<T>& v, const Property& p)
	{
		LOG_WARN("Vectors are not implented yet on XML Scene Load. Skipped loading vector: ", p.GetNameStr());
	}

	template<typename T>
	void operator()(T& v, const Property& p)
	{
		LOG_WARN("Unimplemented property: {}", refl::GetName<T>());
	}
};

} // namespace

void Node::LoadReflectedProperties(const tinyxml2::XMLElement* xmlData)
{
	using namespace parsingaux;

	LoadPropertiesFromXMLVisitor visitor;
	visitor.xmlData = xmlData;
	visitor.worldHandle = Engine::GetWorld()->GetLoadedFromHandle();
	refltools::CallVisitorOnEveryProperty(this, visitor);
}
