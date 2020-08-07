#include "pch.h"
#include "EdClipboardOp.h"

#include "ecs_universe/ComponentsDb.h"

#include "reflection/ReflectionTools.h"

#include <nlohmann/json.hpp>
#include <GLFW/glfw3.h>

namespace ed {
namespace {
	void SetClipboard(const char* clp) { glfwSetClipboardString(nullptr, clp); }
	void SetClipboard(const std::string& clp) { SetClipboard(clp.c_str()); }

	const char* GetClipboard() { return glfwGetClipboardString(nullptr); }
} // namespace


void ClipboardOp::StoreEntity(Entity ent)
{
	nlohmann::json j;
	ComponentsDb::EntityHierarchyToJson(*ent.registry, ent.entity, j);
	SetClipboard(j.dump());
}

Entity ClipboardOp::LoadEntity(entt::registry& reg)
{
	nlohmann::json j = nlohmann::json::parse(GetClipboard(), nullptr, false);
	if (j.is_discarded()) {
		return {};
	}
	return ComponentsDb::JsonToEntityHierarchy(reg, j);
}

void ClipboardOp::StoreStructure(void* data, const ReflClass& cl)
{
	using namespace refltools;

	nlohmann::json j;
	ToJsonVisitor v(j);

	CallVisitorOnEveryPropertyEx(data, cl, v);
	SetClipboard(j.dump());
}

void ClipboardOp::LoadStructure(void* data, const ReflClass& cl)
{
	using namespace refltools;

	nlohmann::json j = nlohmann::json::parse(GetClipboard(), nullptr, false);

	if (j.is_discarded()) {
		return;
	}

	JsonToPropVisitor_WorldLoad visitor(j);
	CallVisitorOnEveryPropertyEx(data, cl, visitor);
}

} // namespace ed
