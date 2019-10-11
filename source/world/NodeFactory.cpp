#include "pch/pch.h"

#include "world/NodeFactory.h"
#include "world/nodes/sky/SkyCubeNode.h"
#include "world/nodes/sky/SkyHDRNode.h"
#include "world/nodes/camera/WindowCameraNode.h"
#include "world/nodes/MetaNodeTranslation.h"
#include "world/World.h"
#include "world/nodes/user/freeform/FreeformUserNode.h"
#include "core/reflection/ReflectionTools.h"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

void NodeFactory::RegisterNodes()
{
	RegisterNodeList<CameraNode, WindowCameraNode, GeometryNode, DirectionalLightNode, PunctualLightNode, SpotLightNode,
		SkyCubeNode, SkyHDRNode, FreeformUserNode, TransformNode>();
}

Node* NodeFactory::NewNodeFromType(const std::string& type)
{
	auto it = m_nodeEntries.find(std::string(FilterNodeName(type)));
	if (it != m_nodeEntries.end()) {
		return it->second.newInstance();
	}

	LOG_ASSERT("Failed to find node registration for: {}", type);
	return nullptr;
}

void NodeFactory::LoadChildren(const nlohmann::json& jsonArray, Node* parent)
{
	for (auto& childObj : jsonArray) {
		CLOG_ASSERT(!childObj.is_object(), "Expected an object.");
		LoadNodeAndChildren(childObj, parent);
	}
}

void NodeFactory::LoadNodeAndChildren(const json& jsonObject, Node* parent)
{
	static const std::string typeLabel = "+type";
	static const std::string nameLabel = "+name";
	static const std::string trsLabel = "+trs";
	static const std::string childrenLabel = "~children";

	CLOG_ASSERT(!jsonObject.is_object(), "Expected a json object here.");


	std::string type = jsonObject.value<std::string>(typeLabel, "");

	if (type.empty()) {
		LOG_WARN("Skipped loading a node without type.");
		return;
	}

	Node* node = NewNodeFromType(type);

	node->m_type = type;
	node->m_name = jsonObject.value<std::string>(nameLabel, "unnamed_" + type);

	auto it = jsonObject.find(trsLabel);
	if (it != jsonObject.end()) {
		LoadNode_Trs(*it, node);
	}

	LoadNode_Properties(jsonObject, node);
	Engine::GetWorld()->RegisterNode(node, parent);
	node->m_dirty.set();
	node->SetDirty(0);

	auto itChildren = jsonObject.find(childrenLabel);
	if (itChildren != jsonObject.end()) {
		LoadChildren(*itChildren, node);
	}
}

namespace glm {
void from_json(const json& j, glm::vec3& vec)
{
	auto it = j.find("x");
	if (it != j.end()) {
		vec.x = j.value<float>("x", 0.f);
		vec.y = j.value<float>("y", 0.f);
		vec.z = j.value<float>("z", 0.f);
		return;
	}
	// Search for rgb mode
	vec.x = j.value<float>("r", 0.f);
	vec.y = j.value<float>("g", 0.f);
	vec.z = j.value<float>("b", 0.f);
}
void from_json(const json& j, glm::vec4& vec)
{
	auto it = j.find("x");
	if (it != j.end()) {
		vec.x = j.value<float>("x", 0.f);
		vec.y = j.value<float>("y", 0.f);
		vec.z = j.value<float>("z", 0.f);
		vec.w = j.value<float>("w", 0.f);
		return;
	}

	vec.x = j.value<float>("r", 0.f);
	vec.y = j.value<float>("g", 0.f);
	vec.z = j.value<float>("b", 0.f);
	vec.z = j.value<float>("a", 0.f);
}
} // namespace glm
template<typename T>
void from_json(const json& j, PodHandle<T>& handle)
{
	if (j.is_string()) {
		handle = AssetManager::GetOrCreate<T>(j.get<std::string>())
	}
}
void from_json(const json& j, MetaEnumInst& enumInst)
{
	if (j.is_string()) {
		enumInst.SetValueByStr(j.get<std::string>());
	}
	else if (j.is_number_integer()) {
		enumInst.SetValue(j.get<enum_under_t>());
	}
}
void NodeFactory::LoadNode_Trs(const nlohmann::json& jsonTrsObject, Node* nodeToLoadInto)
{
	static const std::string posLabel = "pos";
	static const std::string rotLabel = "rot";
	static const std::string scaleLabel = "scale";
	static const std::string lookatLabel = "lookat";

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
		node->m_localOrientation = utl::GetOrientationFromLookAtAndPosition(lookat, node->GetLocalTranslation());
		return;
	}

	auto eulerPyr = j.value<glm::vec3>(rotLabel, {});
	node->m_localOrientation = glm::quat(glm::radians(eulerPyr));
}

namespace {
using json = nlohmann::json;
// TODO: move to independant file.
struct JsonToPropVisitor {

	const json& j;
	PropertyFlags::Type flagsToSkip;

	JsonToPropVisitor(const json& inJson, PropertyFlags::Type inFlagsToSkip = PropertyFlags::NoLoad)
		: j(inJson)
		, flagsToSkip(inFlagsToSkip)
	{
	}

	bool PreProperty(const Property& p)
	{
		if (p.HasFlags(flagsToSkip)) {
			return false;
		}
		return true;
	}

	template<typename T>
	void operator()(T& v, const Property& prop)
	{
		auto it = j.find(prop.GetName());
		if (it != j.end()) {
			it->get_to<T>(v);
		}
	}
};

// With Parent path override and conditionally gltfPreload
struct JsonToPropVisitor_WithRelativePath {

	JsonToPropVisitor parentVisitor;
	BasePodHandle podParent;
	bool gltfPreload;

	JsonToPropVisitor_WithRelativePath(const json& inJson, BasePodHandle inParent, bool inGltfPreload = false,
		PropertyFlags::Type inFlagsToSkip = PropertyFlags::NoLoad)
		: parentVisitor(inJson, inFlagsToSkip)
		, podParent(inParent)
		, gltfPreload(inGltfPreload)
	{
	}

	// forward any non ovreloaded call to parent
	template<typename T>
	void operator()(T& v, const Property& prop)
	{
		parentVisitor.operator()(v, prop);
	}

	template<typename T>
	PodHandle<T> LoadHandle(const std::string& str)
	{
		return AssetManager::GetOrCreateFromParent<T>(str, podParent);
	}

	template<>
	PodHandle<ModelPod> LoadHandle(const std::string& str)
	{
		auto handle = AssetManager::GetOrCreateFromParent<ModelPod>(str, podParent);
		if (gltfPreload && str.find(".gltf") != std::string::npos) {
			AssetManager::PreloadGltf(AssetManager::GetPodUri(handle) + "{}");
		}
		return handle;
	}

	template<typename T>
	void operator()(PodHandle<T>& p, const Property& prop)
	{
		auto it = parentVisitor.j.find(prop.GetName());
		if (it == parentVisitor.j.end()) {
			LOG_ASSERT("Failed to find pod property: {}", prop.GetName());
		}
		p = LoadHandle<T>(it->get<std::string>());
	}

	template<typename T>
	void operator()(std::vector<PodHandle<T>>& v, const Property& prop)
	{
		auto it = parentVisitor.j.find(prop.GetName());
		if (it != parentVisitor.j.end() && it->is_array()) {
			for (auto& elem : *it) {
				v.emplace_back(LoadHandle<T>(elem.get<std::string>()));
			}
		}
	}
};
} // namespace

void NodeFactory::LoadNode_Properties(const nlohmann::json& j, Node* node)
{
	refltools::CallVisitorOnEveryProperty(
		node, JsonToPropVisitor_WithRelativePath(j, Engine::GetWorld()->GetLoadedFromHandle(), true));
}
