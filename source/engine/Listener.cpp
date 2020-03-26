#include "pch.h"

#include "engine/Listener.h"
#include "engine/MulticastEvent.h"

Listener::~Listener()
{
	for (auto& event : m_boundEvents) {
		event->UnbindFromDestructor(this);
	}
}
