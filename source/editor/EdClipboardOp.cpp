#include "EdClipboardOp.h"

#include "universe/ComponentsDb.h"

#include "reflection/ReflectionTools.h"

#include <nlohmann/json.hpp>
#include <GLFW/glfw3.h>

namespace ed {
namespace {
	void SetClipboard(const char* clp) {}
	void SetClipboard(const std::string& clp)
	{
		std::string clipboard = "```" + clp + "```";
		glfwSetClipboardString(nullptr, clipboard.c_str());
	}

	const char* GetClipboard()
	{
		const char* clipboard = glfwGetClipboardString(nullptr);
		if (!clipboard) {
			return nullptr;
		}
		std::string_view sv = clipboard;

		if (sv.starts_with("```")) {
			clipboard += 3;
		}
		if (sv.ends_with("```")) {
			const_cast<char*>(clipboard)[sv.size() - 6] = '\0';
		}
		return clipboard;
	}
} // namespace


void ClipboardOp::StoreEntity(Entity ent)
{
	nlohmann::json j;
	ComponentsDb::EntityHierarchyToJson(*ent.registry, ent.entity, j);
	SetClipboard(j.dump());
}

Entity ClipboardOp::LoadEntity(entt::registry& reg)
{
	auto clipboard = GetClipboard();
	if (!clipboard) {
		return {};
	}
	nlohmann::json j = nlohmann::json::parse(clipboard, nullptr, false);
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

	auto clipboard = GetClipboard();
	if (!clipboard) {
		return;
	}

	nlohmann::json j = nlohmann::json::parse(clipboard, nullptr, false);

	if (j.is_discarded()) {
		return;
	}

	JsonToPropVisitor_WorldLoad visitor(j);
	CallVisitorOnEveryPropertyEx(data, cl, visitor);
}

} // namespace ed
