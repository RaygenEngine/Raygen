#include "Entity.h"

#include "universe/BasicComponent.h"
#include "universe/ComponentsDb.h"

BasicComponent* Entity::operator->()
{
	return &Get<BasicComponent>();
}

void Entity::Destroy()
{
	auto& basic = Get<BasicComponent>();
	basic.SetParent();

	ComponentsDb::VisitWithType(*this, [&](const ComponentMetaEntry& ent) { ent.markDestroy(*registry, entity); });

	auto current = basic.firstChild;
	while (current) {
		Entity next = current->next;
		current.Destroy();
		current = next;
	}

	registry->get_or_emplace<CDestroyFlag>(entity);
}
