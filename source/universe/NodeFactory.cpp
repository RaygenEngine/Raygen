#include "pch.h"
#include "NodeFactory.h"

#include "assets/util/ParsingUtl.h"
#include "core/MathUtl.h"
#include "reflection/ReflectionTools.h"
#include "universe/nodes/camera/CameraNode.h"
#include "universe/nodes/camera/WindowCameraNode.h"
#include "universe/nodes/geometry/GeometryNode.h"
#include "universe/nodes/light/AmbientNode.h"
#include "universe/nodes/light/DirectionalLightNode.h"
#include "universe/nodes/light/PunctualLightNode.h"
#include "universe/nodes/light/SpotLightNode.h"
#include "universe/nodes/TransformNode.h"
#include "universe/nodes/user/FlyingUserNode.h"
#include "universe/nodes/user/FreeformUserNode.h"
#include "universe/Universe.h"

#include <nlohmann/json.hpp>


using json = nlohmann::json;

void NodeFactory::RegisterNodes()
{
	RegisterListToFactory<CameraNode, WindowCameraNode, GeometryNode, DirectionalLightNode, PunctualLightNode,
		SpotLightNode, AmbientNode, FreeformUserNode, TransformNode, FlyingUserNode>();
}

Node* NodeFactory::NewNodeFromType(const std::string& type)
{
	auto it = std::find_if(begin(m_nodeEntries), end(m_nodeEntries),
		[type](auto& other) { return str::equalInsensitive(sceneconv::FilterNodeClassName(type), other.first); });

	if (it != m_nodeEntries.end()) {
		return it->second.newInstance();
	}

	LOG_ABORT("Failed to find node registration for: {}", type);
	return nullptr;
}

void NodeFactory::LoadChildren(const nlohmann::json& jsonArray, Node* parent)
{
	for (auto& childObj : jsonArray) {
		CLOG_ABORT(!childObj.is_object(), "Element in children array is not a json object. Parent was: {} | {}",
			parent->GetClass().GetNameStr(), parent->GetName());
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
	Universe::MainWorld->Z_RegisterNode(node, parent);


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

	node->m_localPosition = j.value<glm::vec3>(posLabel, {});
	node->m_localScale = j.value<glm::vec3>(scaleLabel, { 1.f, 1.f, 1.f });

	auto it = j.find(lookatLabel);
	if (it != j.end()) {
		auto lookat = j.value<glm::vec3>(lookatLabel, {});
		node->m_localOrientation = math::findOrientation(lookat, node->GetNodePositionLCS());
		return;
	}

	auto eulerPyr = j.value<glm::vec3>(rotLabel, {});
	node->m_localOrientation = glm::quat(glm::radians(eulerPyr));
}


void NodeFactory::LoadNode_Properties(const nlohmann::json& j, Node* node)
{
	auto local = refltools::JsonToPropVisitor_WithRelativePath(
		j, Universe::MainWorld->GetLoadedFromPath().generic_string(), true);

	refltools::CallVisitorOnEveryProperty(node, local);
}
