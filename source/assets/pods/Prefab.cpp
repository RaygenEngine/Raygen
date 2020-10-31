#include "Prefab.h"

#include "universe/ComponentsDb.h"

#include <nlohmann/json.hpp>

void Prefab::InsertInto(entt::registry& into) const
{
	if (data.empty()) {
		return;
	}
	nlohmann::json j;

	std::stringstream stream(data);
	stream >> j;
	if (j.empty()) {
		return;
	}
	ComponentsDb::JsonToEntityHierarchy(into, j);
}

void Prefab::MakeFrom(entt::registry& reg, const entt::entity& entity)
{
	nlohmann::json j;

	ComponentsDb::EntityHierarchyToJson(reg, entity, j);

	std::stringstream stream;
	stream << j;

	data = stream.str();
}
