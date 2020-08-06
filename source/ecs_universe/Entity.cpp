#include "pch.h"
#include "Entity.h"

#include "ecs_universe/BasicComponent.h"

BasicComponent* Entity::operator->()
{
	return &Get<BasicComponent>();
}
