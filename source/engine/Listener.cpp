#include "pch.h"
#include "Listener.h"

#include "engine/MulticastEvent.h"

Listener::~Listener()
{
	for (auto& event : m_boundEvents) {
		event->UnbindFromDestructor(this);
	}
}
