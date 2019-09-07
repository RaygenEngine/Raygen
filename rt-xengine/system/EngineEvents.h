#pragma once
#include "event/Event.h"

#define DECLARE_EVENT_LISTENER(EventName) decltype(Event::EventName)::Listener EventName{ Event::EventName }

namespace Event
{
	inline MulticastEvent<int32, int32> OnWindowResize;
}
