#include "pch.h"
#include "ComponentsDb.h"

#include "universe/BasicComponent.h"
#include "reflection/ReflectionTools.h"

#include <nlohmann/json.hpp>
#include <iostream>

void ComponentsDb::EntityHierarchyToJson(entt::registry& reg, entt::entity ent, nlohmann::json& json)
{
	using namespace refltools;

	auto& bs = reg.get<BasicComponent>(ent);

	// Basic component info
	{
		auto& jbasic = json["+"];
		jbasic["name"] = bs.name;
		jbasic["trs"]
			= { { "pos", bs.local().position }, { "euler_rot", bs.local().pyr() }, { "scl", bs.local().scale } };
	}

	// Children
	{
		json["~"] = json::array();
		auto& jchildren = json["~"];

		auto current = bs.firstChild;

		while (current) {
			EntityHierarchyToJson(reg, current.entity, jchildren.emplace_back(json::object()));
			current = current->next;
		}
	}

	// Write the rest of the components
	reg.visit(ent, [&](entt::id_type type) {
		if (auto comp = GetType(type); comp) {
			auto& jcomp = json[comp->clPtr->GetNameStr()];

			ToJsonVisitor visitor(jcomp);
			CallVisitorOnEveryPropertyEx(comp->get(reg, ent), *comp->clPtr, visitor);
		}
	});
}

void ComponentsDb::LoadComponentInto(Entity ent, const std::string& componentName, const nlohmann::json& json)
{
	if (auto comp = GetTypeByName(componentName); comp) {
		comp->emplace(*ent.registry, ent.entity);
		refltools::JsonToPropVisitor_WorldLoad visitor(json);
		refltools::CallVisitorOnEveryPropertyEx(comp->get(*ent.registry, ent.entity), *comp->clPtr, visitor);
		return;
	}

	LOG_ERROR(
		"Component with type: {} was found but currently not registered to database. (Json written with different "
		"engine version? Component wa renamed?)",
		componentName);
}

Entity ComponentsDb::JsonToEntityHierarchy(entt::registry& reg, const nlohmann::json& json)
{
	using namespace refltools;

	if (!json.is_object()) {
		LOG_ERROR("Json provided to JsonToEntityHierarchy was not an object, skipping load.");
		return {};
	}

	auto it = json.find("+");
	if (it == json.end()) {
		LOG_ERROR("Json provided to JsonToEntityHierarchy was missing '+' metadata entry, skipping load.");
		return {};
	}

	auto& jbasic = *it;


	Entity ent{ reg.create(), &reg };
	auto& basic = ent.Add<BasicComponent>();
	basic.self = ent;
	basic.name = jbasic["name"];

	// Load Local transform
	{
		if (auto trsIt = jbasic.find("trs"); trsIt != jbasic.end()) {
			auto& j = *trsIt;
			basic.local_.position = j.value<glm::vec3>("pos", {});
			basic.local_.scale = j.value<glm::vec3>("scl", { 1.f, 1.f, 1.f });

			auto eulerPyr = j.value<glm::vec3>("euler_rot", {});
			basic.local_.orientation = glm::quat(glm::radians(eulerPyr));
			basic.local_.Compose();
			basic.UpdateWorldTransforms();
		}
	}

	for (auto& [key, value] : json.items()) {
		if (key == "+") {
			continue;
		}
		if (key == "~") {
			for (auto& jchild : value.get<json::array_t>()) {
				JsonToEntityHierarchy(reg, jchild)->SetParent(ent, false);
			}
			continue;
		}

		Get().LoadComponentInto(ent, key, value);
	}
	return ent;
}

void ComponentsDb::RegistryToJson(entt::registry& reg, nlohmann::json& json)
{
	reg.each([&](entt::entity entity) {
		auto& bs = reg.get<BasicComponent>(entity);

		// Only do root entities, the rest will be done recursively by their respective parent
		if (!bs.parent) {
			EntityHierarchyToJson(reg, entity, json.emplace_back(nlohmann::json::object()));
		}
	});
}

void ComponentsDb::JsonToRegistry(const nlohmann::json& json, entt::registry& reg)
{
	if (!json.is_array()) {
		LOG_ERROR("JsonToRegistry called with json that was not an array, skipping load");
		return;
	}

	if (!reg.empty()) {
		LOG_ERROR("JsonToRegistry called with registry that not empty, clearing it before load.");
		reg.clear();
	}

	for (auto& jchild : json.get<nlohmann::json::array_t>()) {
		JsonToEntityHierarchy(reg, jchild);
	}
}
