#include "pch.h"

#include "engine/Object.h"
#include "engine/MulticastEvent.h"

Object::~Object()
{
	for (auto& event : m_boundEvents) {
		event->UnbindFromDestructor(this);
	}
}
