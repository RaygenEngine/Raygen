#include "pch.h"

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

Node::Node(Node* pNode)
		: m_localTranslation(0.f, 0.f, 0.f),
		m_localOrientation(1.f, 0.f, 0.f, 0.f),
		// note w, x, y, z
		m_localScale(1.f, 1.f, 1.f),
		m_localMatrix(glm::mat4(1.f)),
		m_worldTranslation(0.f, 0.f, 0.f),
		m_worldOrientation(1.f, 0.f, 0.f, 0.f),
		m_worldScale(1.f, 1.f, 1.f),
		m_worldMatrix(glm::mat4(1.f)),
		m_dirty(),
	    m_parent(pNode)
{
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
	
	glm::vec3 skew; glm::vec4 persp;
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
	for (auto& child : m_children)
	{
		child->MarkMatrixChanged();
	}
}

void Node::UpdateTransforms(const glm::mat4& parentMatrix)
{

	if (m_dirty[DF::TRS])
	{
		m_localMatrix = utl::GetTransformMat(m_localTranslation, m_localOrientation, m_localScale);
		m_worldMatrix = parentMatrix * m_localMatrix;
		// PERF:
		glm::vec3 skew; glm::vec4 persp;
		glm::decompose(m_worldMatrix, m_worldScale, m_worldOrientation, m_worldTranslation, skew, persp);
	}



	for (auto& uPtr : m_children)
	{
		uPtr->UpdateTransforms(m_worldMatrix);
	}
}

void Node::DeleteChild(Node* child)
{
	auto it = std::find_if(m_children.begin(), m_children.end(), [&](auto& ref) 
	{
		return ref.get() == child;
	});
	m_children.erase(it);
	m_dirty.set(DF::Children);
}

bool Node::LoadFromXML(const tinyxml2::XMLElement* xmlData)
{
	ParsingAux::ReadFillEntityName(xmlData, m_name);
	ParsingAux::ReadFillEntityType(xmlData, m_type);
	
	LOG_INFO("Loading {0} named {1}", m_type, m_name);

	LoadReflectedProperties(xmlData);
	LoadAttributesFromXML(xmlData);

	NodeFactory* factory = Engine::GetWorld()->GetNodeFactory();
	const auto status = factory->LoadChildrenXML(xmlData, this);
	
	m_dirty.set();
	
	return status;
}

bool Node::LoadAttributesFromXML(const tinyxml2::XMLElement * xmlData)
{
	ParsingAux::ReadFloatsAttribute(xmlData, "translation", m_localTranslation);
	glm::vec3 eulerPYR{ 0.f, 0.f, 0.f };
	ParsingAux::ReadFloatsAttribute(xmlData, "euler_pyr", eulerPYR);
	m_localOrientation = glm::quat(glm::radians(eulerPYR));
	ParsingAux::ReadFloatsAttribute(xmlData, "scale", m_localScale);

	glm::vec3 localLookat{};
	if (ParsingAux::ReadFloatsAttribute(xmlData, "lookat", localLookat))
	{
		// if lookat read overwrite following
		m_localOrientation = utl::GetOrientationFromLookAtAndPosition(localLookat, GetLocalTranslation());
	}

	return true;
}

void Node::AddLocalOffset(const glm::vec3& direction)
{
	m_localTranslation += direction;
	MarkMatrixChanged();
}

// TODO: orient speed adjust by world delta?
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

namespace 
{

	struct LoadPropertiesFromXMLVisitor
	{
		const tinyxml2::XMLElement* xmlData;
		const char* str;
		std::string strBuf;
		PodHandle<XMLDocPod> worldHandle;

		bool PreProperty(const Property& p)
		{
			if (p.HasFlags(PropertyFlags::NoLoad))
			{
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

		void operator()(glm::vec3& v, const Property& p)
		{
			ParsingAux::ReadFloatsAttribute(xmlData, str, v);
		}
		void operator()(glm::vec4& v, const Property& p)
		{
			ParsingAux::ReadFloatsAttribute(xmlData, str, v);
		}

		void operator()(std::string& v, const Property& p)
		{
			ParsingAux::ReadStringAttribute(xmlData, str, v);
		}

		// Pod<T>
		template<typename T>
		void operator()(PodHandle<T>& handle, const Property& p)
		{
			std::string tmp;
			if (!ParsingAux::ReadStringAttribute(xmlData, str, tmp))
			{
				if (!p.HasFlags(PropertyFlags::OptionalPod))
				{
					LOG_ASSERT("Failed to load non optional pod: {} (Attribute missing in scene file).", p.GetName());
				}
			}
			handle = AssetManager::GetOrCreateFromParent<T>(tmp, worldHandle);
		}
	

		void operator()(PodHandle<ModelPod>& handle, const Property& p)
		{
			std::string tmp;
			if (!ParsingAux::ReadStringAttribute(xmlData, str, tmp))
			{
				if (!p.HasFlags(PropertyFlags::OptionalPod))
				{
					LOG_ASSERT("Failed to load non optional pod: {} (Attribute missing in scene file).", p.GetName());
				}
			}
			handle = AssetManager::GetOrCreateFromParent<ModelPod>(tmp, worldHandle);
		
			if (tmp.find(".gltf") != std::string::npos)
			{
				AssetManager::PreloadGltf(AssetManager::GetPodUri(handle));
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

}

void Node::LoadReflectedProperties(const tinyxml2::XMLElement* xmlData)
{
	using namespace ParsingAux;

	LoadPropertiesFromXMLVisitor visitor;
	visitor.xmlData = xmlData;
	visitor.worldHandle = Engine::GetWorld()->GetLoadedFromHandle();
	refltools::CallVisitorOnEveryProperty(this, visitor);
}
