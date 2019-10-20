#include "pch/pch.h"

#include "world/NodeFactory.h"
#include "world/World.h"
#include "world/nodes/camera/CameraNode.h"
#include "world/nodes/camera/WindowCameraNode.h"
#include "world/nodes/geometry/GeometryNode.h"
#include "world/nodes/light/DirectionalLightNode.h"
#include "world/nodes/light/SpotLightNode.h"
#include "world/nodes/light/PunctualLightNode.h"
#include "world/nodes/sky/SkyboxNode.h"
#include "world/nodes/user/FreeformUserNode.h"
#include "world/nodes/TransformNode.h"
#include "asset/util/ParsingAux.h"
#include "reflection/ReflectionTools.h"
#include "core/MathAux.h"

#include <nlohmann/json.hpp>


using json = nlohmann::json;

void NodeFactory::RegisterNodes()
{
	RegisterNodeList<CameraNode, WindowCameraNode, GeometryNode, DirectionalLightNode, PunctualLightNode, SpotLightNode,
		SkyboxNode, FreeformUserNode, TransformNode>();
}

Node* NodeFactory::NewNodeFromType(const std::string& type)
{
	auto it = std::find_if(begin(m_nodeEntries), end(m_nodeEntries), [type](auto& other) {
		return smath::CaseInsensitiveCompare(sceneconv::FilterNodeClassName(type), other.first);
	});

	if (it != m_nodeEntries.end()) {
		return it->second.newInstance();
	}

	LOG_ABORT("Failed to find node registration for: {}", type);
	return nullptr;
}

void NodeFactory::LoadChildren(const nlohmann::json& jsonArray, Node* parent)
{
	for (auto& childObj : jsonArray) {
		CLOG_ABORT(!childObj.is_object(), "Expected an object.");
		LoadNodeAndChildren(childObj, parent);
	}
}

void NodeFactory::LoadNodeAndChildren(const json& jsonObject, Node* parent)
{
	using namespace sceneconv; // Use scene conventions form parsing aux

	CLOG_ABORT(!jsonObject.is_object(), "Expected a json object here.");

	std::string type = jsonObject.value<std::string>(typeLabel, "");

	if (type.empty()) {
		LOG_WARN("Skipped loading a node without type.");
		return;
	}

	Node* node = NewNodeFromType(type);

	node->m_name = jsonObject.value<std::string>(nameLabel, "unnamed_" + type);

	auto it = jsonObject.find(trsLabel);
	if (it != jsonObject.end()) {
		LoadNode_Trs(*it, node);
	}

	LoadNode_Properties(jsonObject, node);
	Engine::GetWorld()->RegisterNode(node, parent);


	auto itChildren = jsonObject.find(childrenLabel);
	if (itChildren != jsonObject.end()) {
		LoadChildren(*itChildren, node);
	}
}

void NodeFactory::LoadNode_Trs(const nlohmann::json& jsonTrsObject, Node* nodeToLoadInto)
{
	using namespace sceneconv;
	auto& node = nodeToLoadInto;
	auto& j = jsonTrsObject;

	if (!j.is_object()) {
		return;
	}

	node->m_localTranslation = j.value<glm::vec3>(posLabel, {});
	node->m_localScale = j.value<glm::vec3>(scaleLabel, { 1.f, 1.f, 1.f });

	auto it = j.find(lookatLabel);
	if (it != j.end()) {
		auto lookat = j.value<glm::vec3>(lookatLabel, {});
		node->m_localOrientation = math::OrientationFromLookatAndPosition(lookat, node->GetLocalTranslation());
		return;
	}

	auto eulerPyr = j.value<glm::vec3>(rotLabel, {});
	node->m_localOrientation = glm::quat(glm::radians(eulerPyr));
}


void NodeFactory::LoadNode_Properties(const nlohmann::json& j, Node* node)
{
	auto local = refltools::JsonToPropVisitor_WithRelativePath(j, Engine::GetWorld()->GetLoadedFromHandle(), true);

	refltools::CallVisitorOnEveryProperty(node, local);
}
