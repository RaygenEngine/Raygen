#include "Prefab.h"

#include "universe/World.h"
#include "universe/ComponentsDb.h"

void Prefab::InsertInto(World& into) const
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

void Prefab::MakeFrom(Entity entity)
{
	nlohmann::json j;

	ComponentsDb::EntityHierarchyToJson(entity, j);

	std::stringstream stream;
	stream << j;

	data = stream.str();
}
