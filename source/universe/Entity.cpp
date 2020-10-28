#include "Entity.h"

#include "universe/BasicComponent.h"
#include "universe/ComponentsDb.h"
#include "universe/World.h"

Entity::Entity(entt::entity entity_, World* world_, entt::registry& registry_)
	: entity(entity_)
	, registry(&registry_)
	, world(world_)
{
}

BasicComponent* Entity::operator->()
{
	return &Get<BasicComponent>();
}

void Entity::Destroy()
{
	auto& basic = Get<BasicComponent>();
	basic.SetParent();

	ComponentsDb::VisitWithType(*this, [&](const ComponentMetaEntry& ent) { ent.markDestroy(*this); });

	auto current = basic.firstChild;
	while (current) {
		Entity next = current->next;
		current.Destroy();
		current = next;
	}

	registry->get_or_emplace<CDestroyFlag>(entity);
}
