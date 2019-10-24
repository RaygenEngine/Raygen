#pragma once

#include "core/Event.h"

#define DECLARE_EVENT_LISTENER(Name, EventName)                                                                        \
	decltype(EventName)::Listener Name { EventName }


class Node;
class CameraNode;
namespace Event {
// int32 width, int32 height
inline MulticastEvent<int32, int32> OnWindowResize;

inline MulticastEvent<> OnWorldLoaded;
inline MulticastEvent<Node*> OnWorldNodeAdded;
inline MulticastEvent<Node*> OnWorldNodeRemoved;
inline MulticastEvent<CameraNode*> OnWorldActiveCameraChanged;
} // namespace Event
