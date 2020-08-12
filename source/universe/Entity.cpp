#include "pch.h"
#include "Entity.h"

#include "universe/BasicComponent.h"

BasicComponent* Entity::operator->()
{
	return &Get<BasicComponent>();
}
