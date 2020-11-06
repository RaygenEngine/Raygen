#include "ComponentsDb.h"

#include "reflection/ReflectionTools.h"
#include "universe/World.h"


void ComponentsDb::EntityHierarchyToJson(Entity ent, nlohmann::json& json)
{
	using namespace refltools;

	auto& bs = ent.Basic();

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
			EntityHierarchyToJson(current, jchildren.emplace_back(json::object()));
			current = current->next;
		}
	}

	// Write the rest of the components
	ent.registry->visit(ent.entity, [&](entt::id_type type) {
		if (auto comp = GetType(type); comp) {
			auto& jcomp = json[comp->clPtr->GetNameStr()];

			ToJsonVisitor visitor(jcomp);
			CallVisitorOnEveryPropertyEx(comp->get(ent), *comp->clPtr, visitor);
		}
	});
}

void ComponentsDb::LoadComponentInto(Entity ent, const std::string& componentName, const nlohmann::json& json)
{
	if (auto comp = GetTypeByName(componentName); comp) {
		comp->emplace(ent);
		refltools::JsonToPropVisitor_WorldLoad visitor(json);
		refltools::CallVisitorOnEveryPropertyEx(comp->get(ent), *comp->clPtr, visitor);
		return;
	}

	LOG_ERROR(
		"Component with type: {} was found but currently not registered to database. (Json written with different "
		"engine version? Component wa renamed?)",
		componentName);
}

Entity ComponentsDb::JsonToEntityHierarchy(World& world, const nlohmann::json& json)
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


	Entity ent = world.CreateEntity(jbasic["name"]);
	auto& basic = ent.Basic();

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
				JsonToEntityHierarchy(world, jchild)->SetParent(ent, false);
			}
			continue;
		}

		Get().LoadComponentInto(ent, key, value);
	}
	return ent;
}

void ComponentsDb::RegistryToJson(World& world, nlohmann::json& json)
{
	world.reg.each([&](entt::entity entity) {
		auto& bs = world.reg.get<BasicComponent>(entity);

		// Only do root entities, the rest will be done recursively by their respective parent
		if (!bs.parent) {
			EntityHierarchyToJson(bs.self, json.emplace_back(nlohmann::json::object()));
		}
	});
}

void ComponentsDb::JsonToRegistry(const nlohmann::json& json, World& world)
{
	if (!json.is_array()) {
		LOG_ERROR("JsonToRegistry called with json that was not an array, skipping load");
		return;
	}

	if (!world.reg.empty()) {
		LOG_ERROR("JsonToRegistry called with registry that not empty, clearing it before load.");
		world.reg.clear();
	}

	for (auto& jchild : json.get<nlohmann::json::array_t>()) {
		JsonToEntityHierarchy(world, jchild);
	}
}

void ComponentsDb::ClearDirties(World& reg)
{
	for (auto func : Get().m_clearFuncs) {
		func(reg.reg);
	}
}
